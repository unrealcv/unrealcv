function response = request(cmd)
    % import java.io.*

    global client_socket;
    if isempty(client_socket)
      connect();
    end

    send_message(client_socket, cmd)
    message = [];
    while isempty(message) % Set timeout
        message = read_message(client_socket);
        % fprintf('Got response %s\n', message);
        % disp(message);
    end
    response = message;
end

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

function send_message(socket, message)
    import java.io.*
    output_stream = socket.getOutputStream;
    data_output_stream = DataOutputStream(output_stream);
    % magic = hex2dec('9E2B83C1');
    magic_bytes = typecast(uint32(hex2dec('9E2B83C1')), 'int8');
    for i = 1:4
        data_output_stream.writeByte(magic_bytes(i));
    end

    payload_len = length(message);
    len_bytes = typecast(uint32(payload_len), 'int8');
    for i = 1:4
      data_output_stream.writeByte(len_bytes(i));
    end

    data_output_stream.writeBytes(char(message));
end
