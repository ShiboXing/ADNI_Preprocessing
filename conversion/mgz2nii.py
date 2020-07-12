import argparse as argp
import os 

def dive(root):
    print(root)
    for f in os.listdir(root):
        path = rf'{root}/{f}'
        print(path)
        if f.endswith('.mgz'):
<<<<<<< HEAD
            nii_path = convert(root, f)
#             clip_space(nii_path)
=======
            convert(root, f)
>>>>>>> parent of 64ab524... feat: add empty space clipping
        elif os.path.isdir(path):
            dive(path)
            
def convert(root, f):
    os.system(rf'mri_convert {root}/{f} {root}/image.nii')

if __name__ == '__main__':
    parser = argp.ArgumentParser(description='convert .mgz files to .nii')
    parser.add_argument('path', type=str, nargs=1)
    args = parser.parse_args()
    dive(args.path[0])
    