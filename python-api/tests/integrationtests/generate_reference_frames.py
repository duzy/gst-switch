from compare import GenerateReferenceFrames
from test_controller import TestSetCompositeMode, TestAdjustPIP

def set_composite_mode_ref_frames(mode):

    test = 'composite_mode_{0}'.format(mode)
    video = 'output-{0}.data'.format(mode)
    gen = GenerateReferenceFrames(test, video)
    gen.generate_frames()

def adjust_pip_ref_frames(index):

    test = 'adjust_pip_{0}'.format(index)
    video = 'output-{0}.data'.format(index)
    gen = GenerateReferenceFrames(test, video)
    gen.generate_frames()


def main():

    test1 = TestSetCompositeMode()
    for i in range(4):
        test1.set_composite_mode(i, True)
        set_composite_mode_ref_frames(i)


    test2 = TestAdjustPIP()
    d = [
                [50, 75, 0, 0],
        ]
    for i in range(4, 5):
        test2.adjust_pip(d[i-4][0], d[i-4][1], d[i-4][2], d[i-4][3], i, True)
        adjust_pip_ref_frames(i)




if __name__ == '__main__':
    main()
