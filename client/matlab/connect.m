function connect(host, port)
    import java.net.Socket
    if nargin == 0
      host = 'localhost';
      port = 9000;
    end

    fprintf('Try connecting to %s:%d\n', ...
            host, port);

    global client_socket;
    if ~isempty(client_socket)
        client_socket.close
    end
    client_socket = Socket(host, port);

    fprintf('Connected to server\n');
    hello_message = [];
    while isempty(hello_message)
        hello_message = read_message(client_socket);
        disp(hello_message);
    end
end

% Is there a way to share code between connect and request?
function payload = read_message(socket)
    import java.io.*
    input_stream   = socket.getInputStream;
    data_input_stream = DataInputStream(input_stream);
    bytes_available = input_stream.available;
    if (bytes_available == 0)
        payload = '';
        return
    end

    bytes = zeros(1, 4, 'int8');
    for i = 1:4
        bytes(i) = data_input_stream.readByte;
    end
    assert(hex2dec('9E2B83C1') == typecast(int8(bytes), 'uint32'));

    for i = 1:4
        bytes(i) = data_input_stream.readByte;
    end
    payload_len = typecast(int8(bytes), 'uint32');
    % disp(payload_len);

    bytes = zeros(1, payload_len);
    for i = 1:payload_len
        bytes(i) = data_input_stream.readByte;
    end
    payload = char(bytes);
end
