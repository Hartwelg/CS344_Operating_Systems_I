#!/usr/bin/python
import string
import random
import sys
def main():
	randomNames = ['hartwelg_file1', 'hartwelg_file2', 'hartwelg_file3']
	counter = 0
	randomLetters = ""
	for i in range(3):
		file = open(randomNames[counter], "w+")
		for i in range(10):
			randomLetters = randomLetters + random.choice(string.ascii_lowercase)
		file.write(randomLetters)
		file.write("\n")
		print(randomLetters)
		randomLetters = ""
		file.close()
		counter = counter + 1
	firstNum = random.randint(1, 42)
	secondNum = random.randint(1, 42)
	print(firstNum)
	print(secondNum)
	finalNum = firstNum * secondNum
	print(finalNum) 
main()