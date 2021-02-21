import os
import random
fold = "C:\\Users\\Hansol Lee\\Downloads\\PyTorch-YOLOv3-master\\data\\custom2\\images"
fi_lst = os.listdir(fold)


fi2_lst = []

for i in fi_lst:
    fi2_lst.append("data/custom2/images/"+i)


random.shuffle(fi2_lst)

fi3_lst = fi2_lst[:int(len(fi2_lst)*0.9)]
fi4_lst = fi2_lst[int(len(fi2_lst)*0.7):]

print(len(fi2_lst), len(fi3_lst)+len(fi4_lst))


w2 = open("train2.txt", "w")

w3 = open("valid2.txt", "w")

for i in fi3_lst:
    w2.write(i+'\n')

for i in fi4_lst:
    w3.write(i+'\n')

w2.close()
w3.close()
