mac_binary='../../RealisticRendering-Darwin-0.3.8/MacNoEditor/RealisticRendering.app/'
start_mac()
{
    open ${mac_binary} &
}

stop_mac()
{
    pkill RealisticRendering
}

start_docker()
{
    nvidia-docker run --name rr --rm -p 9000:9000 --env="DISPLAY" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
        qiuwch/rr:0.3.8 > rr.log &
    sleep 2
}

stop_docker()
{
    docker stop rr
}

$@
