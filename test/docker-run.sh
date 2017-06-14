if [ -z "$1" ]; then
    script="pytest connection_test.py \
        camera_test.py \
        stereo_test.py \
        rr_test.py"
else
    script="$@"
fi
echo 'Test to run : ' ${script}

rr_image=qiuwch/rr:0.3.8
docker_run() {
    docker pull ${rr_image}
    nvidia-docker run --name rr --rm -p 9000:9000 --env="DISPLAY" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
        ${rr_image} > rr.log &
    sleep 3
    $@
    docker stop rr
}

if type nvidia-docker; then
    docker_run ${script}
else
    ${script}
fi
