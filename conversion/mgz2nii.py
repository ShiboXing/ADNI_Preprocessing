%pylab inline
import argparse as argp
import os 
import matplotlib.pyplot as plt

def dive(root):
    print(root)
    for f in os.listdir(root):
        path = rf'{root}/{f}'
        print(path)
        if f.endswith('.mgz'):
            nii_path = convert(root, f)
            clip_space(nii_path)
        elif os.path.isdir(path):
            dive(path)
            
def convert(root, f):
    os.system(rf'mri_convert {root}/{f} {root}/image.nii')
    return rf'mri_convert {root}/{f} {root}/image.nii'

# remove the surrounding zero-intensity space from the brain
def clip_boundary(path):
    img = nib.load(path)
    fig, axes = plt.subplots(1, len(slices))
    slice_0 = epi_img_data[26, :, :]
    axes[0].imshow(slice.T, cmap="gray", origin="lower")

if __name__ == '__main__':
    parser = argp.ArgumentParser(description='convert .mgz files to .nii')
    parser.add_argument('path', type=str, nargs=1)
    args = parser.parse_args()
    dive(args.path[0])
    