script=$1
nvidia-docker run --name rr --rm -p 9000:9000 --env="DISPLAY" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    qiuwch/rr:0.3.8 > rr.log &
sleep 3
pytest ${script}
docker stop rr

