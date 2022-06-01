import sys
import subprocess

def main():
	for path in sys.stdin:
		pwd = '/home/{}'.format(str(path).strip())
		p1 = subprocess.Popen('ls -a {}'.format(pwd), shell=True, stdout=subprocess.PIPE)
		files = str(p1.communicate()[0])
		
		if files.find('.cache') > -1:
			print('-a never,exit -F dir=' + pwd + '/.cache')
		if files.find('.mozilla') > -1:
			print('-a never,exit -F dir=' + pwd + '/.mozilla')
		if files.find('.vscode-server') > -1:
			print('-a never,exit -F dir=' + pwd + '/.vscode-server')

if __name__ == '__main__':
	main()
