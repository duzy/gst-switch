import os
import cv2
import sys

from scipy.misc import imread
from scipy.linalg import norm
from scipy import sum, average

import subprocess

def check_video(video, cmp_folder):

    
    generate_keyframes(video, cmp_folder)

    result = {}
    for x in os.listdir('reference_frames'):
        try:
            res = comp_image('reference_frames/'+x, 'test_frames/'+x)
            # if res:
            result[x] = res
        except Exception as e:
            pass
    for x in result:
        print x, result[x]


def generate_keyframes(video, cmp_folder):
    cmd = "/usr/local/bin/ffmpeg -i {0} -f image2 -vf fps=fps=1 {1}/foo-%d.png".format(video, cmp_folder)
    # print cmd
    with open(os.devnull, 'w') as tempf:
            proc = subprocess.Popen(
            cmd.split(),
            stdout=tempf,
            stderr=tempf,
            bufsize=-1,
            shell=False)
            proc.wait()


def to_grayscale(arr):
    "If arr is a color image (3D array), convert it to grayscale (2D array)."
    if len(arr.shape) == 3:
        return average(arr, -1)  # average over the last axis (color channels)
    else:
        return arr

def comp_image(image1, image2):
    file1, file2 = (image1, image2)
    # read images as 2D arrays (convert to grayscale for simplicity)
    img1 = imread(file1).astype(float)
    img2 = imread(file2).astype(float)
    # compare
    n_0 = compare_images(img1, img2)
    return n_0*1.0/img1.size
    # print "Manhattan norm:", n_m, "/ per pixel:", n_m/img1.size
    # print "Zero norm:", n_0, "/ per pixel:", n_0*1.0/img1.size

def normalize(arr):
    rng = arr.max()-arr.min() + 1e-10
    amin = arr.min()
    return (arr-amin)*255/rng


def compare_images(img1, img2):
    # normalize to compensate for exposure difference, this may be unnecessary
    # consider disabling it
    img1 = normalize(img1)
    img2 = normalize(img2)
    # calculate the difference and its norms
    diff = img1 - img2  # elementwise for scipy arrays
    # m_norm = sum(abs(diff))  # Manhattan norm
    z_norm = norm(diff.ravel(), 0)  # Zero norm
    return z_norm

def main():
    video1 = 'rec1.data'
    video = 'rec2.data'
    generate_keyframes(video1, 'reference_frames')
    print check_video(video, 'test_frames')

main()

