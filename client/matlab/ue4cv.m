function ue4cv()
    import java.net.Socket
    import java.io.*

    host = 'localhost';
    port = 9000;

    try
        fprintf('Try connecting to %s:%d\n', ...
                host, port);

        client_socket = Socket(host, port);
        input_stream   = client_socket.getInputStream;
        data_input_stream = DataInputStream(input_stream);

        fprintf('Connected to server\n');
        bytes_available = input_stream.available;
        message = zeros(1, bytes_available, 'uint8');
        for i = 1:bytes_available
            message(i) = data_input_stream.readByte;
        end

        message = char(message);
        disp(message);

        client_socket.close;

    catch exception
        disp(exception);
        fprintf('Fail to connect to server\n');
        if ~isempty(client_socket)
            client_socket.close;
        end
    end
end

function read_message(socket)
  magic = socket.readByte;
  payload_len = socket.readByte;
end

function send_message(socket, message)

end
