function queue1_callback(msg)
   debug(ERROR, 'Callback called')
end

debug(ERROR, 'Registering callback for queue1')

register_callback('queue1', 'queue1_callback')

debug(ERROR, 'Initialized test script')

