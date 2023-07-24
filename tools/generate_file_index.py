import os


def gci(filepath):
    files = os.listdir(filepath)
    for fi in files:
        fi_d = os.path.join(filepath,fi)
        if os.path.isdir(fi_d):
            gci(fi_d)
        else:
            print('"{}", '.format(os.path.relpath(os.path.normpath(fi_d), '../res')), end = '')

if __name__ == '__main__':
    gci('../res')
