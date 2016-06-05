function ue4cv()
    import java.net.Socket
    import java.io.*

    host = 'localhost';
    port = 9000;

%     try
        fprintf('Try connecting to %s:%d\n', ...
                host, port);

        global client_socket;
        if ~isempty(client_socket)
            client_socket.close
        end
        client_socket = Socket(host, port);
        pause(2);

        fprintf('Connected to server\n');
        message = read_message(client_socket);
        disp(message);
%         message = char(byte_message);
%         disp(message);
        send_message(client_socket, 'hello')
        pause(1);
        message = read_message(client_socket);
        disp(message);

%         client_socket.close; % Make sure to release the connection in any case.

%     catch exception
%         disp(exception);
%         fprintf('Fail to connect to server\n');
%         if ~isempty(client_socket)
%             client_socket.close;
%         end
%     end
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

%     magic = socket.readByte;
%     payload_len = socket.readByte;
%
%     bytes_available = input_stream.available;
%         byte_message = zeros(1, bytes_available, 'int8');
%         for i = 1:bytes_available
%             byte = data_input_stream.readByte;
%             byte_message(i) = byte;
%         end

    bytes = zeros(1, 4, 'int8');
    for i = 1:4
        bytes(i) = data_input_stream.readByte;
    end
    assert(hex2dec('9E2B83C1') == typecast(int8(bytes), 'uint32'));

    for i = 1:4
        bytes(i) = data_input_stream.readByte;
    end
    payload_len = typecast(int8(bytes), 'uint32');
    disp(payload_len);

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
    magic = hex2dec('9E2B83C1');
    magic_bytes = typecast(uint32(hex2dec('9E2B83C1')), 'int8');
%     data_output_stream.writeBytes(magic_bytes);
%     data_output_stream.writeInt(
    for i = 1:4
        data_output_stream.writeByte(magic_bytes(i));
    end
    payload_len = length(message);
    len_bytes = typecast(uint32(payload_len), 'int8');
    for i = 1:4
      data_output_stream.writeByte(len_bytes(i));
    end
% data_output_stream.writeInt(payload_len);
    data_output_stream.writeBytes(message)
end
