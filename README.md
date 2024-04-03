## Ray Tracer

Created using C++, in Visual Studio.

This program generates images of objects with realistic lighting by simulating how light behaves in a virtual environment. This project is a more up-to-date recreation of a previous ray tracer that I worked on: https://github.com/nvasiu/Ray-Tracer. This project's features include:
* Diffuse shading and realistic shadows.
* Rough or reflective surfaces.
* Light emitting objects.
* A menu UI that lets users control the variables of the environment.
* Real time image rendering.
* (Transparent objects and light refraction are a work in progress.)

| ![Screenshot 2024-03-26 145609](https://github.com/nvasiu/RayTracer/assets/46430801/909bf32f-0546-4443-a33b-1fc8b9a3931a) |
|:--:| 
| **Example Image With Program UI** |

| ![Screenshot 2024-03-26 150641](https://github.com/nvasiu/RayTracer/assets/46430801/ac1f4c8c-ad62-4af9-aef6-6fdfc9f5662c) |
|:--:| 
| **Reflective, Rough and Light Emitting Spheres** |

This program simulates realistic lighting in real time by randomly modifying the direction that light rays bounce. This randomness means that images will initially be generated with some amount of noise (as seen below). Letting the program run with the "accumulate" option turned on will gather color data over many render iterations and average them out, resulting in a cleaner image.

| ![Screenshot 2024-03-26 151233](https://github.com/nvasiu/RayTracer/assets/46430801/3bdb006a-e52a-4f0b-9a9b-9601e34f72e0) |
|:--:| 
| **How An Image Looks Initially Without Accumulation** |

| ![Screenshot 2024-03-26 151209](https://github.com/nvasiu/RayTracer/assets/46430801/d5310199-7216-40e2-bdb4-c602c9e12533) |
|:--:| 
| **How The Previous Image Looks After A Second Of Accumulation** |

| ![image](https://github.com/nvasiu/RayTracer/assets/46430801/eb5a04b8-2323-443e-986e-0af9d081dd87) |
|:--:| 
| **Work In Progress Of Transparent Objects** |
