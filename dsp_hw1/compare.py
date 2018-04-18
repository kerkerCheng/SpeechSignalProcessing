import numpy as np

predict = np.loadtxt('acc_test1.txt', dtype=np.str)
ans = np.loadtxt('testing_answer.txt', dtype=np.str)

length = ans.shape[0]

correct = 0
for i in range(length):
    if predict[i] == ans[i]:
        correct += 1

print(correct/length)
