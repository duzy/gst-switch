from test_server import TestServerStartStop
import os
import glob
import time


def test_server_start_stops():
    fails = []
    for i in range(5):
        try:
            TestServerStartStop(i)
            time.sleep(3)
        except OSError:
            fails.append(i)

    print fails

def remove_data_files():
    for f1 in glob.glob(os.getcwd()+'/*.data'):
        os.remove(f1)




def main():
    test_server_start_stops()
    remove_data_files


if __name__== "__main__":
    main()