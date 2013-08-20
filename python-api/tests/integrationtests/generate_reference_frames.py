from compare import CompareVideo

def set_composite_mode_ref_frames(mode):
    test = 'composite_mode_{0}'.format(mode)
    video = 'output-{0}.data'.format(mode)
    cmpr = CompareVideo(test, video)
    cmpr.generate_frames()


def main():
    for i in range(4):
        set_composite_mode_ref_frames(i)

if __name__ == '__main__':
    main()
