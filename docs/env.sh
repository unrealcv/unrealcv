
mac_binary='../../RealisticRendering-Darwin-0.3.8/MacNoEditor/RealisticRendering.app/'
start_mac()
{
  open ${mac_binary} &
}

stop_mac()
{
  pkill RealisticRendering
}

$@
