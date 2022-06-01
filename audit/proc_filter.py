import sys

def main():
	for line in sys.stdin:
		print('-a never,exit -S all -F ppid=' + str(line).strip())
		print('-a never,exit -S all -F pid=' + str(line).strip())

if __name__ == '__main__':
	main()
