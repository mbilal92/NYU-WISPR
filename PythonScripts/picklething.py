
import os
import pickle
import pandas as pd


def main():
	df = pd.read_pickle("/home/ubuntu/Downloads/Wifi frames/10.1.1.11_DF.pickle")
	print df


if __name__ == '__main__':
    main()