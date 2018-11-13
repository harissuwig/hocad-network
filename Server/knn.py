# Example of kNN implemented from Scratch in Python
# Code from https://machinelearningmastery.com/tutorial-to-implement-k-nearest-neighbors-in-python-from-scratch/

import sys
import csv
import random
import math
import operator

def loadDataset(filename, trainingSet=[]):
	with open(filename, 'rb') as csvfile:
	    lines = csv.reader(csvfile)
	    dataset = list(lines)
	    for x in range(len(dataset)):
	    	for y in range(2):			
				dataset[x][y] = float(dataset[x][y])
	    	trainingSet.append(dataset[x])


def euclideanDistance(instance1, instance2, length):
	distance = 0
	for x in range(length):
		distance += pow((instance1 - instance2[x]), 2)
	return math.sqrt(distance)

def getNeighbors(trainingSet, testInstance, k):
	distances = []
	length = 1
	for x in range(len(trainingSet)):
		dist = euclideanDistance(testInstance, trainingSet[x], length)
		distances.append((trainingSet[x], dist))

	distances.sort(key=operator.itemgetter(1))
	neighbors = []
	for x in range(k):
		neighbors.append(distances[x][0])
	return neighbors

def getResponse(neighbors):
	classVotes = {}
	for x in range(len(neighbors)):
		response = neighbors[x][-1]
		if response in classVotes:
			classVotes[response] += 1
		else:
			classVotes[response] = 1
	sortedVotes = sorted(classVotes.iteritems(), key=operator.itemgetter(1), reverse=True)
	return sortedVotes[0][0]

def getAccuracy(testSet, predictions):
	correct = 0
	for x in range(len(testSet)):
		if testSet[x][-1] == predictions[x]:
			correct += 1
	return (correct/float(len(testSet))) * 100.0
	
def main(file_name, k, input_data):
	# prepare data
	trainingSet=[]
	loadDataset(file_name, trainingSet)
	neighbors = getNeighbors(trainingSet, input_data, k)
	result = getResponse(neighbors)
	print int(result)
	
if __name__ == "__main__":
    if len(sys.argv) > 3:
        try:
            file_name = str(sys.argv[1])
            k = int(sys.argv[2])
            input_data = float(sys.argv[3])
            main(file_name, k, input_data)
        except IOError as e:
            print("File not found.")
	
