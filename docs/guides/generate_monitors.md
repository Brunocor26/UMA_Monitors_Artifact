# How to build and generate rmtld3synth from docker container
 
 ## Run the container

docker run -it "anmaped/rmtld3synth"


 ## Inside the container CLI (this example for this specific condition):

rmtld3synth --synth-cpp11 --input-dsl "duration of a in 0 .. 15 >= 5" --out-src out

## Check the running container image id

docker ps

## Copy the output files from that docker image to your desired folder

docker cp <image_id>:/root/out <path>/output