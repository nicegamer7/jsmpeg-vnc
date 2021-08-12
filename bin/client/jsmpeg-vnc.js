var displayParameter = new URLSearchParams(window.location.search).get('display');
var displayUpdating = false;

var localCursorX = 0;
var localCursorY = 0;

var handleSocketMessage = function(source, message) {
    if (document.hidden) {
        return;
    }

    data = new Uint8Array(message.data).subarray(12);
    view = new DataView(message.data);

    var display = view.getInt32(0, true);
    var x = view.getInt32(4, true);
    var y = view.getInt32(8, true);

    if (displayUpdating) {
        displayUpdating = (display != displayParameter);
        if (!displayUpdating) {
            source.video.clearSequenceHeader();
        }
    }

    if ((x != localCursorX) || (y != localCursorY)) {
        localCursorX = x;
        localCursorY = y;

        var rect = canvas.getBoundingClientRect();
        var scaleX = canvas.width / (rect.right - rect.left);
        var scaleY = canvas.height / (rect.bottom - rect.top);

        cursor.style.left = canvas.style.left + ((localCursorX / scaleX) + rect.left - (cursor.clientWidth / 2)) + 'px';
        cursor.style.top = canvas.style.top + ((localCursorY / scaleY) + rect.top - (cursor.clientHeight / 2)) + 'px';
    }

    return data;
}

var handleWindowResize = function() {
    canvas.style.width = (canvas.width > window.innerWidth) ? '100%' : canvas.width + 'px';
    canvas.style.height = (canvas.height > window.innerHeight) ? '100%' : canvas.height + 'px';
}

function getCookie(name) {
    var v = document.cookie.match('(^|;) ?' + name + '=([^;]*)(;|$)');
    return v ? v[2] : null;
}

var canvas = document.getElementById('video-canvas');
var cursor = document.getElementById('cursor');

var password = new URLSearchParams(window.location.search).get('password');

var inputPort = parseInt(window.location.port) + 1;
var inputURL = 'ws://' + window.location.hostname + ':' + inputPort + '/' + password;
var inputSocket = new WebSocket(inputURL, 'ws');
inputSocket.binaryType = 'arraybuffer';

var playerURL = 'ws://' + window.location.host + '/' + password;
var player = new JSMpeg.Player(playerURL, {
    canvas: canvas,
    protocols: 'ws',
    reconnectInterval: null,
    audio: false,
    videoBufferSize: getCookie('videoBufferSize'),
    onVideoDecodeSequenceHeader: handleWindowResize,
    onSocketMessage: handleSocketMessage
});

const MESSAGE_DISPLAY = 0;
const MESSAGE_PASTE = 1;
const MESSAGE_COPY = 2;
const MESSAGE_KEY_DOWN = 3;
const MESSAGE_KEY_UP = 4;
const MESSAGE_MOUSE_MOVE = 5;
const MESSAGE_MOUSE_LEFT_DOWN = 6;
const MESSAGE_MOUSE_LEFT_UP = 7;
const MESSAGE_MOUSE_RIGHT_DOWN = 8;
const MESSAGE_MOUSE_RIGHT_UP = 9;
const MESSAGE_MOUSE_MIDDLE_DOWN = 10;
const MESSAGE_MOUSE_MIDDLE_UP = 11;
const MESSAGE_MOUSE_SCROLL = 12;
const MESSAGE_UPLOAD_FILE = 13;
const MESSAGE_CURSOR_POSITION = 14;

inputSocket.onopen = function() {
    if (displayParameter != null) {
        sendDisplay(displayParameter);
    }
}

inputSocket.onmessage = function(event) {
    var data = new DataView(event.data);
    var message = data.getInt32(0, true);

    switch (message) {
        case MESSAGE_DISPLAY: {
            displayParameter = data.getInt32(4, true);
            displayUpdating = true;
        }
        break;

        case MESSAGE_COPY: {
            var data = new Uint8Array(event.data).subarray(4);
            var clipboard = new TextDecoder('utf-8').decode(data);
            var element = document.getElementById('clipboard');

            if (element == null) {
                element = document.createElement('textarea');
                element.id = 'clipboard';
                element.style.display = 'none';

                document.body.appendChild(element);
            }

            element.style.display = 'block';
            element.value = clipboard;
            element.select();

            document.execCommand('copy');

            element.style.display = 'none';
        }
        break;
    }
}

var send = function(data) {
    if (document.title === 'VNC') { // Not uploading
        inputSocket.send(data);
    }
}

var sendKey = function(event, action, key) {
    event.preventDefault();

    var message = new ArrayBuffer(8);
    var type = new Uint32Array(message, 0);
    var code = new Uint32Array(message, 4);

    type[0] = action;
    code[0] = key;

    send(message);
};

var sendMousePosition = function(event) {
    event.preventDefault();

    var message = new ArrayBuffer(24);
    var type = new Uint32Array(message, 0);
    var x = new Float64Array(message, 8);
    var y = new Float64Array(message, 16);

    var rect = canvas.getBoundingClientRect();

    type[0] = MESSAGE_MOUSE_MOVE;
    x[0] = (event.clientX - rect.left) / rect.width;
    y[0] = (event.clientY - rect.top) / rect.height;

    send(message);
};

var sendMouseButton = function(event, button) {
    event.preventDefault();

    var message = new ArrayBuffer(4);
    var type = new Uint32Array(message, 0);

    type[0] = button;

    send(message);
}

var sendMouseScroll = function(event) {
    event.preventDefault();

    var message = new ArrayBuffer(8);
    var type = new Uint32Array(message, 0);
    var amount = new Int32Array(message, 4);

    type[0] = MESSAGE_MOUSE_SCROLL;
    amount[0] = event.wheelDelta;

    send(message);
}

var sendCopy = function() {
    send(new Uint32Array([MESSAGE_COPY]));
}

var sendDisplay = function(display) {
    send(new Uint32Array([MESSAGE_DISPLAY, display]));
}

window.addEventListener('keydown', function(event) {
    if (event.ctrlKey && event.keyCode === 86) {
        return false;
    }

    sendKey(event, MESSAGE_KEY_DOWN, event.keyCode);

    if (event.ctrlKey && event.keyCode === 67) {
        sendCopy();
    }
}, false);

window.addEventListener('keyup', function(event) {
    if (event.ctrlKey && event.keyCode === 86) {
        return false;
    }

    sendKey(event, MESSAGE_KEY_UP, event.keyCode);
}, false);

canvas.addEventListener('mousemove', function(event) {
    sendMousePosition(event);
}, false);

canvas.addEventListener('mousedown', function(event) {
    switch (event.button) {
        case 0:
            sendMouseButton(event, MESSAGE_MOUSE_LEFT_DOWN);
            break;
        case 1:
            sendMouseButton(event, MESSAGE_MOUSE_MIDDLE_DOWN);
            break;
        case 2:
            sendMouseButton(event, MESSAGE_MOUSE_RIGHT_DOWN);
            break;
    }
}, false);

canvas.addEventListener('mouseup', function(event) {
    switch (event.button) {
        case 0:
            sendMouseButton(event, MESSAGE_MOUSE_LEFT_UP);
            break;
        case 1:
            sendMouseButton(event, MESSAGE_MOUSE_MIDDLE_UP);
            break;
        case 2:
            sendMouseButton(event, MESSAGE_MOUSE_RIGHT_UP);
            break;
    }
}, false);

canvas.addEventListener('mousewheel', function(event) {
    sendMouseScroll(event);
}, false);

window.addEventListener('paste', function(event) {
    if (event.preventDefault()) {
        event.preventDefault();
    }

    var clipboard = new TextEncoder().encode(event.clipboardData.getData('text/plain') + '\0');

    var messageSize = clipboard.length + 4;
    while ((messageSize % 4) != 0) {
        messageSize++;
    }

    var message = new ArrayBuffer(messageSize);
    var type = new Uint32Array(message, 0);
    var contents = new Uint8Array(message, 4);

    type[0] = MESSAGE_PASTE;
    contents.set(clipboard);

    send(message)

    sendKey(event, MESSAGE_KEY_DOWN, 17);
    sendKey(event, MESSAGE_KEY_DOWN, 86);

    sendKey(event, MESSAGE_KEY_UP, 17);
    sendKey(event, MESSAGE_KEY_UP, 86);
});

window.addEventListener('drop', function(ev) {
    if (ev.preventDefault()) {
        ev.preventDefault();
    }

    var reader = new FileReader();

    reader.onerror = function(e) {
        alert('File type not supported for uploading. You likely want to archive your files.');
    }

    reader.onload = function(e) {
        var contents = new Uint8Array(reader.result);
        var contentsSize = contents.length;
        contentsSize = contentsSize + (4 - contentsSize % 4);

        var filename = new TextEncoder().encode(reader.fileName + '\0');
        var filenameSize = 256;

        var messageSize = contentsSize + filenameSize + 8;
        var message = new ArrayBuffer(messageSize);
        var messageType = new Uint32Array(message, 0);
        var messageFileName = new Uint8Array(message, 4);
        var messageFileSize = new Uint32Array(message, 260);
        var messageFileContents = new Uint8Array(message, 264);

        messageType.set([MESSAGE_UPLOAD_FILE]);
        messageFileName.set(filename);
        messageFileSize.set([contentsSize]);
        messageFileContents.set(contents);

        send(message);

        var progressInterval = setInterval(function() {
            if (inputSocket.bufferedAmount === 0) {
                clearInterval(progressInterval);
                document.title = 'VNC';
                return;
            }
            var remaining = messageSize - inputSocket.bufferedAmount;
            var percentage = Math.round((remaining * 100) / messageSize);

            document.title = 'VNC - Uploading: ' + percentage + '%';
        }, 500);
    }

    if (ev.dataTransfer.files.length > 0) {
        reader.fileName = ev.dataTransfer.files[0].name;
        reader.readAsArrayBuffer(ev.dataTransfer.files[0]);
    }
});

window.addEventListener('dragover', function(ev) {
    if (ev.preventDefault()) {
        ev.preventDefault();
    }
}, false);

window.addEventListener('resize', function(e) {
    handleWindowResize();
});
