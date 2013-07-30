RayTracer
=========

A ray tracer for 3D sound rendering

* Simulate sound propagation with rays.
* Record sound impulse responses.
* Convolve the generated impulse response with HRTF database(kemar from MIT)
* Compute the new response.

Our sound rendering is based on the binaural recording. Two microphones are placed in the left and right ear canals of a dummy human head model. A loudspeaker gives out impulses from different positions, so the microphones could record the impulse responses. When we had such response, we can convolve the original sound wave with the impulse response, and then the new wave possesses directional information. This is how a 3D sound is generated.

![RayTracer](https://github.com/billhsu/RayTracer/raw/master/doc/br.png)  
![RayTracer](https://github.com/billhsu/RayTracer/raw/master/doc/fig1.jpg)  

An impulse response (it is also called HRTF: head-related transfer function) from MIT KEMAR database.  
What we are trying to do here is to generate 3d sound in a scene: we use the MIT KEMAR database to calculate the impulse responses from a source to a listen in a scene in real-time.  
![RayTracer](https://github.com/billhsu/RayTracer/raw/master/doc/capture.png)  
The red points in the screenshot above are hypothetical sound rays, which record the strength and direction of the sound ray. The scene is an OBJ model. When rays collide with the primitives (triangles), they bounce. The sound ray’s strength decays with distance and bounces.  
![RayTracer](https://github.com/billhsu/RayTracer/raw/master/doc/results.png)  
When we have all the sound rays’ information, we can calculate the scene’s impulse responses by scaling and overlying the MIT KEMAR database.  
![RayTracer](https://github.com/billhsu/RayTracer/raw/master/doc/new_response.png)  
Above is the new impulse response and sound wave we generated in our scene.

##Author
**Shipeng Xu**

+ http://BillHsu.me
+ http://twitter.com/1991bill
+ http://weibo.com/billhsu
