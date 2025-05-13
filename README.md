# Noisy Hue-ligans
Lexi Henrion, Vivian Li, Apoorva Talwalkar, and Sherry Zhang

## Background
This Github repo contains the code and results for our CSCI2240 Advanced Computer Graphics Project.

## Paper Details
For our final project we implemented the **Color Me Noisy: Example-based Rendering of Hand-colored Animations with Temporal Noise Control** paper by researchers **J. Fišer** from CTU in Prague, **M. Lukác** from CTU in Prague, **O. Jamriška** from CTU in Prague, **M. Čadík** from the Brno University of Technology, **Y. Gingold** from George Mason University, **P. Asente** from Adobe Research, and **D. Sýkora** from CTU in Prague. The paper proposes a method for rendering 3D animations with a 2D hand-colored feel by mimicking the textures of a reference texture. You can find the paper at [this link](https://dcgi.fel.cvut.cz/home/sykorad/cmn.html).

## Slides
For a breakdown on our implementation and more results checkout our slides [here](https://docs.google.com/presentation/d/1H5HIQYvSIxh_ptMewWtXSpEcxy8iWtKoKZzU519LR2s/edit?usp=sharing)!

## Results
![](/results_gifs/bird_input_output.gif)

At different Image Pyramid Levels
![](/results_gifs/witch_levels.gif)

Using an Input Texture Very Different fromm the Original
![](/results_gifs/texture_diff.gif)

## Extensions
We tried using our algorithm on a still image and running the image through our model over and over again to see how a the texture varies. We also attempted the extenion idea in the paper for local orientation control (directional textures such as hatching follow a prescribed direction instead of random, but we didn't get it working. Here are our results without orientation control:
![](/results_gifs/eye.gif)

## Credits
We worked on the main loop of the algorithm together as a group. Below is a breakdown of our contributions for other functions and effort  

Lexi: Image pyramids and reconstruction  
Vivian: Bilinear interpolation used in upsample/desample  
Apoorva: Texture deformation and slides  
Sherry: Patchmatch and results video (in slides)  

A special thanks to our professor Daniel Ritchie and TA mentor Stewart Morris for their guidance on this project and all semester!
