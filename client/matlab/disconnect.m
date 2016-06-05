function disconnect()
  global client_socket;
  if ~isempty(client_socket)
    client_socket.close
    client_socket = [];
  end
end
