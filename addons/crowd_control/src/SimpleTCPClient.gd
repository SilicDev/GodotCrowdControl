class_name CCSimpleTCPClient
extends RefCounted

signal socket_connected()
signal request_received(request: CCRequest)

enum Status {
	NONE,
	DISCONNECTED,
	CONNECTING,
	CONNECTED,
	ERROR,
}

var socket: StreamPeerTCP = StreamPeerTCP.new()
var client_lock := Mutex.new()

var connected : bool
var quitting := false
var ready := false
var status : Status = Status.NONE

var _threads: Array[Thread] = []

func connect_to_host(host: String, port: int) -> void:
	if not is_connected_to_host():
		var t := Thread.new()
		t.start(_connect_loop.bind(host, port))
		_threads.append(t)
		t = Thread.new()
		t.start(_listen)
		_threads.append(t)
		t = Thread.new()
		t.start(_keep_alive)
		_threads.append(t)

func _connect_loop(host: String, port: int) -> void:
	OS.set_thread_name(OS.get_processor_name() + ": CrowdControl Connect Loop")
	while not status == Status.DISCONNECTED and not quitting:
		socket = StreamPeerTCP.new()
		status = Status.CONNECTING
		var err := socket.connect_to_host(host, port)
		if err:
			continue
		socket.poll()
		while socket.get_status() == StreamPeerTCP.STATUS_CONNECTING:
			OS.delay_msec(20)
			socket.poll()
		if socket.get_status() != StreamPeerTCP.STATUS_CONNECTED:
			continue
		connected = true
		status = Status.CONNECTED
		ready = true
		socket_connected.emit()
		print("connected")
		while not quitting:
			OS.delay_msec(100)
		socket.disconnect_from_host()
		connected = false
		ready = false
	connected = false


func _listen() -> void:
	OS.set_thread_name(OS.get_processor_name() + ": CrowdControl Listener")
	var msg := PackedByteArray([])
	var json := JSON.new()
	while not quitting:
		OS.delay_msec(20)
		if not ready or not status == Status.CONNECTED:
			continue
		if socket.get_status() != StreamPeerTCP.STATUS_CONNECTED:
			continue
		socket.poll()
		var len := socket.get_available_bytes()
		if len != 0:
			var data : PackedByteArray = socket.get_data(len)[1]
			for b in data:
				if b != 0:
					msg.append(b)
				else:
					var str := msg.get_string_from_utf8()
					var dict := str_to_var(str)
					var request := CCRequest.from_dict(dict)
					request_received.emit(request)
					msg.resize(0)
		if not quitting:
			OS.delay_msec(1000)

func _keep_alive() -> void:
	OS.set_thread_name(OS.get_processor_name() + ": CrowdControl Keep Alive")
	while not quitting:
		if connected:
			var response := CCResponse.new()
			response.id = 0
			response.type = _CrowdControl.ResponseType.KEEP_ALIVE
			respond(response)
		OS.delay_msec(1000)
	if not quitting:
		OS.delay_msec(1000)
	pass

func respond(response: CCResponse) -> bool:
	var json := JSON.new()
	var msg := str(response.to_dict())
	var buffer := msg.to_utf8_buffer()
	buffer.append(0)
	while not client_lock.try_lock():
		OS.delay_msec(20)
	socket.big_endian = true
	var err := socket.put_data(buffer)
	client_lock.unlock()
	if err:
		return false
	return true

func disconnect_from_host() -> void:
	if is_connected_to_host():
		quitting = true
		status == Status.DISCONNECTED
		socket.disconnect_from_host()
		for i in _threads.size():
			print("Joining %s" % i)
			_threads[i].wait_to_finish()
	else:
		push_warning("Must be connected to a host to disconnect!")

func is_connected_to_host() -> bool:
	return status == Status.CONNECTED

class CCRequest:
	var id: int
	var code: String
	var message: String
	var parameters: Array
	var targets: Array[Target]
	var duration: int
	var quantity: int
	var viewer: String
	var viewers: Array[Target]
	var cost: int
	var type: _CrowdControl.RequestType
	
	func to_dict() -> Dictionary:
		var out := {
			"id": id,
			"code": code,
			"message": message,
			"parameters": null,
			"targets": null,
			"duration": duration,
			"quantity": quantity,
			"viewer": viewer,
			"viewers": null,
			"cost": cost,
			"type": type,
		}
		var temp := []
		for p in parameters:
			temp.append(str(p))
		if temp.size() != 0:
			out["parameters"] = temp
		temp = []
		for t in targets:
			temp.append(t.to_dict())
		if temp.size() != 0:
			out["targets"] = temp
		for t in viewers:
			temp.append(t.to_dict())
		if temp.size() != 0:
			out["viewers"] = temp
		return out
	
	static func from_dict(dict: Dictionary) -> CCRequest:
		var out := CCRequest.new()
		out.id = dict.get("id", -1)
		out.code = dict.get("code", "")
		out.message = dict.get("message", "")
		out.viewer = dict.get("viewer", "")
		out.cost = dict.get("cost", 0)
		out.type = dict.get("type", 0)
		var arr = dict.get("parameters", [])
		var arr2 = []
		for p in arr:
			arr2.append(str_to_var(p))
		if arr2.size() != 0:
			out.parameters = arr2
		arr = dict.get("targets", [])
		var arr3 : Array[Target] = []
		for p in arr:
			arr3.append(Target.from_dict(p))
		if arr3.size() != 0:
			out.targets = arr3
		arr = dict.get("viewers", [])
		arr3 = []
		for p in arr:
			arr3.append(Target.from_dict(p))
		if arr3.size() != 0:
			out.viewers = arr3
		return out
	
	class Target:
		var id: String
		var name: String
		var login: String
		var avatar: String
		#var service: CrowdControl.ServiceType #RESERVED
		
		func to_dict() -> Dictionary:
			return {
				"id": id,
				"name": name,
				"login": login,
				"avatar": avatar,
				#"service": service,
			}
	
		static func from_dict(dict: Dictionary) -> Target:
			var out := Target.new()
			out.id = dict.get("id", "")
			out.name = dict.get("name", "")
			out.login = dict.get("login", "")
			out.avatar = dict.get("avatar", "")
			#out.service = dict.get("service", 0)
			return out

class CCResponse:
	var id: int
	var status: _CrowdControl.EffectResult
	var message: String
	var time_remaining: int
	var type: _CrowdControl.ResponseType = _CrowdControl.ResponseType.EFFECT_REQUEST
	
	func to_dict() -> Dictionary:
		return {
			"id": id,
			"status": status,
			"message": message,
			"timeRemaining": time_remaining,
			"type": type,
		}
	
	func to_dict_string() -> String:
		if message != "":
			return '{"id":%d,"status":%d,"message":"%s","timeRemaining":%d,"type":%d}' % [id, status, message, time_remaining, type]
		return '{"id":%d,"status":%d,"timeRemaining":%d,"type":%d}' % [id, status, time_remaining, type]
	
	static func from_dict(dict: Dictionary) -> CCResponse:
		var out := CCResponse.new()
		
		return out
