-- LUA script skel of BSPD
-- By Dr.NP <np@bsgroup.org>

local _data_proto = function(fd, proto)
    print('Client ' .. fd .. ' send proto-only request :');
    --bsp_var_dump(proto);
    bsp_net_send(fd, proto.REQUEST_URI .. " is a BIG SB!!!");
    --bsp_net_close(fd);
end

local _data_stream = function(fd, stream, proto)
    --print('Client ' .. fd .. ' send stream : ' .. stream);
end

local _data_object = function(fd, object, proto)
    --print('Client ' .. fd .. ' send object :');
    bsp_var_dump(object);
end

local _data_command = function(fd, cmd, params, proto)
    --print('Client ' .. fd .. ' send command ' .. cmd .. ' with params :');
    --print(cmd);
    --bsp_var_dump(params);
end

function server_connect(fd)
    print('Client ' .. fd .. ' connect to server');
end

function server_disconnect(fd, session_id)
    print('Client ' .. fd .. ' disconnect from server');
end

function server_data(fd, ...)
    local args = {...};
    local ret = nil;
    if ('nil' == type(args[1]) and 'table' == type(args[2])) then
        -- Just proto
        local proto = args[2];
        ret = _data_proto(fd, proto);
    elseif ('string' == type(args[1])) then
        -- Stream
        local stream = args[1];
        local proto = nil;
        if ('table' == type(args[2])) then
            proto = args[2];
        end

        ret = _data_stream(fd, stream, proto);
    elseif ('table' == type(args[1])) then
        -- Object
        local object = args[1];
        local proto = nil;
        if ('table' == type(args[2])) then
            proto = args[2];
        end

        ret = _data_object(fd, object, proto);
    elseif ('number' == type(args[1]) and 'table' == type(args[2])) then
        -- Comand
        local cmd = args[1];
        local params = args[2];
        local proto = nil;
        if ('table' == type(args[3])) then
            proto = args[3];
        end

        ret = _data_command(fd, params, proto);
    end

    return ret;
end

