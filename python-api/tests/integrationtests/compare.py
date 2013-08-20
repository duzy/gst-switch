import os

from scipy.misc import imread
from scipy.linalg import norm
from scipy import  average

import subprocess


class CompareVideo(object):
    """Compare a video with pre-stored frames
    :parameter test: The test which is conducted.
    self.TESTS stores these tests and maps them with frames
    :parameter video: The video which has to be compared
    """
    TESTS = {
        'composite_mode_0': 1,
        'composite_mode_1': 2,
        'composite_mode_2': 3,
        'composite_mode_3': 4,
    }
    REF_FRAME_DIR = 'reference_frames'
    TEST_FRAME_DIR = 'test_frames'
    def __init__(self, test, video):
        super(CompareVideo, self).__init__()
        self.test = test
        self.video = video

    def generate_frames(self):
        """Generate the original frames for storage in REF_FRAME_DIR
        """
        if not os.path.exists(self.REF_FRAME_DIR):
            os.mkdir(self.REF_FRAME_DIR)
        
        cmd1 = "ffmpeg -i {0} -ss 00:00:02.000 -f image2 -vframes 1 {1}/out{2}_1.png".format(self.video, self.REF_FRAME_DIR, self.TESTS[self.test])
        cmd2 = "ffmpeg -i {0} -ss 00:00:05.000 -f image2 -vframes 1 {1}/out{2}_2.png".format(self.video, self.REF_FRAME_DIR, self.TESTS[self.test])
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

    def  compare(self):
        x = '/out{0}_1.png'.format(self.TESTS[self.test])
        y = '/out{0}_2.png'.format(self.TESTS[self.test])

        if not os.path.exists(self.TEST_FRAME_DIR):
            os.mkdir(self.TEST_FRAME_DIR)
        
        cmd1 = "ffmpeg -i {0} -ss 00:00:02.000 -f image2 -vframes 1 {1}/out{2}_1.png".format(self.video, self.TEST_FRAME_DIR, self.TESTS[self.test])
        cmd2 = "ffmpeg -i {0} -ss 00:00:05.00 -f image2 -vframes 1 {1}/out{2}_2.png".format(self.video, self.TEST_FRAME_DIR, self.TESTS[self.test])
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

        res1 = self.comp_image(self.REF_FRAME_DIR+x, self.TEST_FRAME_DIR+x)
        res2 = self.comp_image(self.REF_FRAME_DIR+y, self.TEST_FRAME_DIR+y)
        return (res1, res2)


    def to_grayscale(arr):
        "If arr is a color image (3D array), convert it to grayscale (2D array)."
        if len(arr.shape) == 3:
            return average(arr, -1)  # average over the last axis (color channels)
        else:
            return arr

    def comp_image(self, image1, image2):
        file1, file2 = (image1, image2)
        # read images as 2D arrays (convert to grayscale for simplicity)
        img1 = imread(file1).astype(float)
        img2 = imread(file2).astype(float)
        # compare
        n_0 = self.compare_images(img1, img2)
        return n_0*1.0/img1.size
        # print "Manhattan norm:", n_m, "/ per pixel:", n_m/img1.size
        # print "Zero norm:", n_0, "/ per pixel:", n_0*1.0/img1.size

    def normalize(self, arr):
        rng = arr.max()-arr.min() + 1e-10
        amin = arr.min()
        return (arr-amin)*255/rng


    def compare_images(self, img1, img2):
        # normalize to compensate for exposure difference, this may be unnecessary
        # consider disabling it
        img1 = self.normalize(img1)
        img2 = self.normalize(img2)
        # calculate the difference and its norms
        diff = img1 - img2  # elementwise for scipy arrays
        # m_norm = sum(abs(diff))  # Manhattan norm
        z_norm = norm(diff.ravel(), 0)  # Zero norm
        return z_norm


