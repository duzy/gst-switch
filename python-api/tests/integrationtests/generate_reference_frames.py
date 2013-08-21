from compare import GenerateReferenceFrames
from test_controller import TestSetCompositeMode

def set_composite_mode_ref_frames(mode):
    test = 'composite_mode_{0}'.format(mode)
    video = 'output-{0}.data'.format(mode)
    gen = GenerateReferenceFrames(test, video)
    gen.generate_frames()


def main():
    test = TestSetCompositeMode()
    for i in range(4):
        test.driver_set_composite_mode(i, True)
        set_composite_mode_ref_frames(i)

if __name__ == '__main__':
    main()
