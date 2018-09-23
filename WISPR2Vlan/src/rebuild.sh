make clean
make
# cp wispr wispr2
# cp wispr wispr3
# cp wispr wispr4
# cp wispr wispr5
# cp wispr wispr6
# cp wispr wispr7

counter=1
while [ $counter -le 180 ]
do
	cp wispr ./wispr0/wispr$counter
	((counter++))
done
