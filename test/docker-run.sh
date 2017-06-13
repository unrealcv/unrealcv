if [ -z $1 ]; then
    script="connection_test.py \
        camera_test.py \
        stereo_test.py \
        rr_test.py"
else
    script=$1
fi
echo 'Test to run : ' ${script}

docker_run() {
    nvidia-docker run --name rr --rm -p 9000:9000 --env="DISPLAY" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
        qiuwch/rr:0.3.8 > rr.log &
    sleep 3
    pytest $@
    docker stop rr
}

docker_run ${script}
