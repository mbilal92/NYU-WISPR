int raw_iphdr_udp4_send(int rsock, char* buf, unsigned int len,
						union sockaddr_union* from,
						union sockaddr_union* to, unsigned short mtu)
{
	struct msghdr snd_msg;
	struct iovec iov[2];
	struct ip_udp_hdr {
		struct iphdr ip;
		struct udphdr udp;
	} hdr;
	unsigned int totlen;
	unsigned int ip_frag_size; /* fragment size */
	unsigned int last_frag_extra; /* extra bytes possible in the last frag */
	unsigned int ip_payload;
	unsigned int last_frag_offs;
	void* last_frag_start;
	int frg_no;
	int ret;

	totlen = len + sizeof(hdr);
	if (unlikely(totlen) > 65535)
		return -2;
	memset(&snd_msg, 0, sizeof(snd_msg));
	snd_msg.msg_name=&to->sin;
	snd_msg.msg_namelen=sockaddru_len(*to);
	snd_msg.msg_iov=&iov[0];
	/* prepare the udp & ip headers */
	mk_udp_hdr(&hdr.udp, &from->sin, &to->sin, (unsigned char*)buf, len, 1);
	mk_ip_hdr(&hdr.ip, &from->sin.sin_addr, &to->sin.sin_addr,
				len + sizeof(hdr.udp), IPPROTO_UDP);
	
	iov[0].iov_base=(char*)&hdr;
	iov[0].iov_len=sizeof(hdr);
	snd_msg.msg_iovlen=2;
	snd_msg.msg_control=0;
	snd_msg.msg_controllen=0;
	snd_msg.msg_flags=0;
	/* this part changes for different fragments */
	/* packets are fragmented if mtu has a valid value (at least an
	   IP header + UDP header fit in it) and if the total length is greater
	   then the mtu */
	if (likely(totlen <= mtu || mtu <= sizeof(hdr))) {
		iov[1].iov_base=buf;
		iov[1].iov_len=len;
		ret=sendmsg(rsock, &snd_msg, 0);
	} else {
		ip_payload = len + sizeof(hdr.udp);
		/* a fragment offset must be a multiple of 8 => its size must
		   also be a multiple of 8, except for the last fragment */
		ip_frag_size = (mtu -sizeof(hdr.ip)) & (~7);
		last_frag_extra = (mtu - sizeof(hdr.ip)) & 7; /* rest */
		frg_no = ip_payload / ip_frag_size +
				 ((ip_payload % ip_frag_size) > last_frag_extra);
		/*ip_last_frag_size = ip_payload % frag_size +
							((ip_payload % frag_size) <= last_frag_extra) *
							ip_frag_size; */
		last_frag_offs = (frg_no - 1) * ip_frag_size;
		/* if we are here mtu => sizeof(ip_h+udp_h) && payload > mtu
		   => last_frag_offs >= sizeof(hdr.udp) */
		last_frag_start = buf + last_frag_offs - sizeof(hdr.udp);
		hdr.ip.id = fastrand_max(65534) + 1; /* random id, should be != 0
											  (if 0 the kernel will fill it) */
		/* send the first fragment */
		iov[1].iov_base = buf;
		/* ip_frag_size >= sizeof(hdr.udp) because we are here only
		   if mtu >= sizeof(hdr.ip) + sizeof(hdr.udp) */
		iov[1].iov_len = ip_frag_size - sizeof(hdr.udp);
		hdr.ip.tot_len = htons(ip_frag_size);
		hdr.ip.frag_off = htons(0x2000); /* set MF */
		
		ret = sendmsg(rsock, &snd_msg, 0);
		if (unlikely(ret < 0))
			goto end;
		
		/* all the other fragments, include only the ip header */
		iov[0].iov_len = sizeof(hdr.ip);
		iov[1].iov_base =  (char*)iov[1].iov_base + iov[1].iov_len;
		
		/* fragments between the first and the last */
		while(unlikely(iov[1].iov_base < last_frag_start)) {
			iov[1].iov_len = ip_frag_size;
			hdr.ip.tot_len = htons(iov[1].iov_len);
			/* set MF  */
			hdr.ip.frag_off = htons( (unsigned short)
									(((char*)iov[1].iov_base - (char*)buf +
										sizeof(hdr.udp)) / 8) | 0x2000);
			ret=sendmsg(rsock, &snd_msg, 0);
			if (unlikely(ret < 0))
				goto end;
			iov[1].iov_base =  (char*)iov[1].iov_base + iov[1].iov_len;
		}

		/* last fragment */
		iov[1].iov_len = buf + len - (char*)iov[1].iov_base;
		hdr.ip.tot_len = htons(iov[1].iov_len);
		/* don't set MF (last fragment) */
		hdr.ip.frag_off = htons( (unsigned short)
								(((char*)iov[1].iov_base - (char*)buf +
									sizeof(hdr.udp)) / 8) );
		ret=sendmsg(rsock, &snd_msg, 0);
		if (unlikely(ret < 0))
			goto end;
	}
end:
	return ret;
}