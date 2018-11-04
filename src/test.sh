rm ../tests/*~
for file in $(ls ../tests):
do
	echo $file
# 	cat ../tests/$file | ./try > op.txt
done
