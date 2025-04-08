# Project 1: Brush

Please fill this out for Brush only. The project handout can be found [here](https://cs1230.graphics/projects/raster/1).

## Output Comparison
This is how you will demonstrate the functionality of your project.

For each of the rows below, follow the instructions to record a video. Please reference the [Screen Recording Guide](https://cs1230.graphics/docs/screen-recording/) for machine-specific guidance on how to do this.

Once you've recorded everything, navigate to this file in Github, click edit, and either select or drag-and-drop each of your videos into the correct location. This will upload them to GitHub (but not include them in the repo) and automatically embed them into this Markdown file by providing a link. Make sure to double-check that they all show up properly in the preview.

> Note: you can ignore the `student_outputs` folder in the repo for now. You will be using it in the next project, Filter.

We're **not** looking for your video to **exactly** match the expected output (i.e. draw the exact same shape). Just make sure to follow the instructions and verify that the recording fully demonstrates the functionality of that part of your project.

### Constant Brush
**Instructions:** Draw with the constant brush.

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/241d99b5-947a-407c-b454-410534520aad

#### Your Output


https://github.com/user-attachments/assets/ca5f75a2-874a-4a87-95aa-8bc6b1926635




### Linear Brush
**Instructions:** Draw with the linear brush.

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/9706fa04-7710-441f-b292-ab010e04dec6

#### Your Output



https://github.com/user-attachments/assets/49463558-24fb-453d-a4e7-4a46ed7452f9




### Quadratic Brush
**Instructions:** Draw with the quadratic brush.

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/c5df5c09-bfe0-4c05-a56e-14609772d675

#### Your Output



https://github.com/user-attachments/assets/ee2d94a6-e178-41b2-836b-4f38ae7c4071





### Smudge Brush
**Instructions:** Draw some colors on the canvas and use the smudge brush to smear them together.

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/26440b63-2d1c-43fd-95f2-55b74ad3bbed

#### Your Output


https://github.com/user-attachments/assets/2145dac2-c773-4f30-b390-faa7aeacbe15





### Smudge Brush Change in Alpha
**Instructions:** Draw some colors on the canvas. Use the smudge brush with varying alpha levels (use at least three) and demonstrate that the brush still works the same way each time.

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/0b49c7d0-47ca-46d0-af72-48b831dfe7ea

#### Your Output


https://github.com/user-attachments/assets/2b2ec526-5633-481c-8c07-5680690869a4





### Radius
**Instructions:** Use any brush with at least 3 different values for the radius.

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/6f619df6-80cd-4849-8831-6a5aade2a517

#### Your Output


https://github.com/user-attachments/assets/e44a5e94-31ca-4198-be54-c78724e65a28





### Color
**Instructions:** Use any brush to draw red (255, 0, 0), green (0, 255, 0), and blue (0, 0, 255).

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/fd9578ca-e0af-433e-ac9e-b27db2ceebc9

#### Your Output

https://github.com/user-attachments/assets/7a028d6a-ab2e-47ca-827e-9d0255a0fe33





### Canvas Edge Behavior
**Instructions:** With any brush, click and draw on the canvas in a place where the mask intersects with the edge. Then, start drawing anywhere on the canvas and drag your mouse off of the edge.

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/f5344248-fa5f-4c33-b6df-ff0a45011c7a

#### Your Output


https://github.com/user-attachments/assets/7013ca17-6385-4bd5-982e-e06b1f704acd




### Alpha
**Instructions:** With the constant brush, draw a single dot of red (255, 0, 0) with an alpha of 255. Then, draw over it with a single dot of blue (0, 0, 255) with an alpha of 100. You should get a purpleish color.

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/b13d312d-d6d4-4375-aeaa-96174065443b

#### Your Output

https://github.com/user-attachments/assets/c75f8c3a-314f-4576-9f1d-3076b7d3f2a6




### Alpha of Zero
**Instructions:** Choose any brush and demonstrate that it will not draw if the alpha value is zero.

#### Expected Output

https://github.com/BrownCSCI1230/projects_raster_template/assets/77859770/8e48777e-8196-401e-9af6-871abe712146

#### Your Output


https://github.com/user-attachments/assets/997922ac-6556-407d-b496-aadbcad35b24




## Design Choices
I stored the brush mask as a 1D vector in the canvas2d file. I thought it would be heavy-handed to store it in a separate file
since all you need to do is access and change values in the vector.

I made a few helper functions (e.g. for converting x,y position to index, checking bounds, checking if colors are the same).

## Collaboration/References
Thanks to Sophie and Praccho at hours!! <3

## Known Bugs
N/A

## Extra Credit
Spray tool and fill work! They are both in canvas2D.cpp. See videos below:

SPRAY: 
https://github.com/user-attachments/assets/df866cd4-aa54-4c80-9f57-aafc4697b37a

FILL:
https://github.com/user-attachments/assets/9b4faa81-ab1b-4998-96d4-c91c64ab471d

Thanks for reading!
<img width="788" alt="ilovegrafix" src="https://github.com/user-attachments/assets/6f4da2fd-b871-4502-ba23-a2d65cdfacb6">

