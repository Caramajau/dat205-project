# DAT205 - Advanced Computer Graphics - Procedural World Project
This is the repo for the procedural world project I did for the [Advanced Computer Graphics course](https://www.student.chalmers.se/sp/course?course_id=40758) at Chalmers University of Technology.

The project is based on the basic project template provided for the course, which can be found at https://gitlab.com/chalmerscg/opengl-project-template.

To build the project, use this command in the root folder: ```cmake -G "Visual Studio 17 2022" -B build``` (The solution file will be found in the build folder)

The original README can be found at the bottom.

## Implemented features
- Generate terrain geometry with layered Perlin noise with different kinds of smoothing operations, seed support and domain warping.
- Achieve some erosion through a "gradient trick" as an extension of the noise, rather than simulating erosion.
- Render the terrain with blended textures (using triplanar mapping) depending on height and slope.
- Flat water with reflection and refraction.
- The generated terrain can be explored through simple movement based on the generated height map.
- Extensive support for customising many parameters to tweak or completely break the features mentioned above.

# Original README file
This is the repository for the basic project template for the Advanced Computer Graphics course given at Chalmers University of
Technology.

You can find instructions on how to build at http://www.cse.chalmers.se/edu/course/TDA362/tutorials/start.html

README_LINUX.md has some specific instructions for linux users.
