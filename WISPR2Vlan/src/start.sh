# sudo ./wispr -i veth0 -o veth2
# sudo ./wispr2 -i veth4 -o veth6
# sudo ./wispr3 -i veth8 -o veth10
# sudo ./wispr4 -i veth12 -o veth14 #-g 00:00:00:00:00:02 #00:50:56:ef:ba:73 #216.58.207.110
# sudo ./wispr5 -i veth16 -o veth18
# sudo ./wispr6 -i veth20 -o veth22
# sudo ./wispr7 -i veth24 -o veth26


#counter=0
#counterWispr=1
#while [ $counter -le 720 ] #720
#do
#	sudo ./wispr0/wispr$counterWispr -i veth$counter -o veth$(($counter+2))
#	echo $counterWispr veth$counter veth$(($counter + 2))
#	((counter++))
#	((counter++))
#	((counter++))
#	((counter++))
#	((counterWispr++))
#done

#echo $counter
#echo $counterWispr

sudo ./wispr0/wispr177 -i veth704 -o veth706
sudo ./wispr0/wispr94 -i veth372 -o veth374

