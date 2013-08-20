import os

from scipy.misc import imread
from scipy.linalg import norm
from scipy import  average

import subprocess



def check_video(video, cmp_folder):

    
    generate_keyframes(video, cmp_folder)
    x = 'out1.png'
    y = 'out2.png'
    res1 = comp_image('reference_frames/'+x, 'test_frames/'+x)
    res2 = comp_image('reference_frames/'+y, 'test_frames/'+y)
    return (res1, res2)

def generate_keyframes(video, cmp_folder):
    cmd1 = "ffmpeg -i {0} -ss 00:00:01.500 -f image2 -vframes 1 {1}/out1.png".format(video, cmp_folder)
    cmd2 = "ffmpeg -i {0} -ss 00:00:04.500 -f image2 -vframes 1 {1}/out2.png".format(video, cmp_folder)
    # print cmd
    with open(os.devnull, 'w') as tempf:
            proc = subprocess.Popen(
            cmd1.split(),
            stdout=tempf,
            stderr=tempf,
            bufsize=-1,
            shell=False)
            proc.wait()
    with open(os.devnull, 'w') as tempf:
            proc = subprocess.Popen(
            cmd2.split(),
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
    video1 = 'input.data'
    video2 = 'input.data'
    if not os.path.exists('reference_frames'):
        os.mkdir('reference_frames')
    if not os.path.exists('test_frames'):
        os.mkdir('test_frames')
    generate_keyframes(video1, 'reference_frames')
    print check_video(video2, 'test_frames')

main()

