/// <reference path="../../node_modules/babylonjs/babylon.module.d.ts" />
/// <reference path="../../node_modules/babylonjs-loaders/babylonjs.loaders.module.d.ts" />
/// <reference path="../../node_modules/babylonjs-materials/babylonjs.materials.module.d.ts" />
/// <reference path="../../node_modules/babylonjs-gui/babylon.gui.module.d.ts" />

var wireframe = false;
var turntable = false;
var logfps = true;
var ibl = false;
var rtt = false;
var vr = false;
var ar = true;
var xrHitTest = false;
var xrFeaturePoints = false;
var meshDetection = false;
var text = false;
var hololens = false;
var cameraTexture = false;
var imageTracking = false;
const readPixels = false;


const RTC = window.navigator.rtc;

class RTCWebsocket {
    constructor(url) {
        this.url = url;
        this.isOpen = false;
        this.isOpening = false;

        RTC.onWebsocketOpen(function(wsData) {
            
            if (wsData["wsUrl"]) {
                this.isOpen = true;
                this.isOpening = false;
            }
        }.bind(this));

        RTC.onWebsocketClosed(function(wsData) {
            BABYLON.Tools.Log("RTC Calling back from ws open rtc check");
            if (wsData["wsUrl"] == this.url) {
                this.isOpen = false;
                this.isOpening = false;
            }
        }.bind(this));
    }

    getOpenState() {
        return this.isOpen;
    }
    getOpeningState() {
        return this.isOpening;
    }
    getUrl() {
        return this.url;
    }

    open() {
        if (!this.isOpen && !this.isOpening) {
            this.isOpening = true;
            RTC.openWebsocket(this.url);
            return true;
        }
        return false;
    }

    close() {
        if (this.isOpen && !this.isOpening) {
            RTC.closeWebsocket(this.url);
            return true;
        }
        return false;
    }

    sendMessage(message) {
        if(this.isOpen && !this.isOpening) {
            RTC.sendWebsocketMessage(this.url, message);
        }
    }

    addOnOpenListener(callback) {
        RTC.onWebsocketOpen(function (wsData) {
            BABYLON.Tools.Log("RTC Calling back from ws open rtc check cb");
            BABYLON.Tools.Log("RTC Calling back from ws open rtc check cb val myUrl=" + this.myUrl);
            if (wsData["wsUrl"] == this.myUrl) {
                BABYLON.Tools.Log("Calling back from ws open");
                this.callback();
            }
        }.bind({myUrl: this.url, callback: callback}));
    }

    addOnCloseListener(callback) {
        RTC.onWebsocketClosed(function (wsData) {
            if (wsData["wsUrl"] == this.myUrl) {
                this.callback();
            }
        }.bind({myUrl: this.url, callback: callback}));
    }

    addOnMessageListener(callback) {
        RTC.onWebsocketMessage(function (wsData) {
            BABYLON.Tools.Log("RTC WS message rcv!");
            if (wsData["wsUrl"] == this.myUrl) {
                BABYLON.Tools.Log("RTC WS message rcv cb!");
                this.callback(wsData["message"]);
            }
        }.bind({myUrl: this.url, callback: callback}));
    }

}

class RTCConnection {

    constructor(identifier, websocket) {

        RTC.setConfig("90.16.68.220"); //custom stun server set

        this.identifier = identifier;
        this.websocket = websocket;

        if (!this.websocket.getOpenState() || !this.websocket.getOpeningState()) {
            this.websocket.open();
        }

        this.remoteDescriptions = {};
        // peerId: [{added: boolean, payload: {sdp: sdp string, type: type string}}]

        this.remoteCandidates = {};
        // peerId: [{added: boolean, payload: {candidate: candidate string, mid: candidate mid string}}]



        this.peerConnectionReady = {};

        
        this.websocketReady = false;

        //TODO: add close/fail callbacks...


        // As of 11/01/2023 ddmmyyyy : does not support instancing multiple RTCConnections...
        // Could be used as static instance in a more standard RTC class abstracting peerId management
        // As we handle low level events and such in the game networking we can use a low level
        // class as this one and handle peerId logic on the event/networking side of the game
        // as we handle it either way.
        RTC.onPeerConnectionCreated(function (pcData) {
            var id = pcData["id"];
            this.peerConnectionReady[id] = true;
            BABYLON.Tools.Log("RTC PC on create! " + id);
            this.flushRemoteDescriptions(id);
            BABYLON.Tools.Log("RTC PC remote candidates! " + id);
            this.flushRemoteCandidates(id);
            BABYLON.Tools.Log("RTC PC remote candidates after! " + id);
        }.bind(this));

        this.dataChannels = {};
        
        RTC.onDataChannel(function (dcData) {
            BABYLON.Tools.Log("RTC DC OPEN f!");
            this.dataChannels[dcData["channelId"]] = true;
            BABYLON.Tools.Log("RTC MSG " + dcData["channelId"] + " is here!");
        }.bind(this));

        RTC.onRemoteDescriptionSet(function (pcData) {
            BABYLON.Tools.Log("RTC onRemoteDescription set " + pcData["id"]);
            this.flushRemoteCandidates(pcData["id"]);
        }.bind(this));

        RTC.onLocalDescription(function (descData) {
            BABYLON.Tools.Log("RTC On local description");
            var type = descData["type"];
            var sdp = descData["sdp"];
    
            //We have to send it over websocket to connect to peer
            if (this.websocketReady) {
                var msgToSend = {
                    "client": this.identifier,
                    "type": type,
                    "payload": {
                        "sdp": sdp,
                    }
                };
                this.websocket.sendMessage(JSON.stringify(msgToSend));
            }
        }.bind(this));
        RTC.onLocalCandidate(function (candidateData) {
            BABYLON.Tools.Log("RTC On local candidate");
            var candidate = candidateData["candidate"];
            var mid = candidateData["mid"];
    
            //We have to send it over websocket to connect to peer
            if (this.websocketReady) {
                var msgToSend = {
                    "client": this.identifier,
                    "type": "candidate",
                    "payload": {
                        "candidate": candidate,
                        "mid": mid,
                    }
                };
                this.websocket.sendMessage(JSON.stringify(msgToSend));
            }
    
        }.bind(this));



        this.websocket.addOnOpenListener(function () {
            BABYLON.Tools.Log("RTC ws add on open rtc conn ok!");
            this.websocketReady = true;
            this.websocket.sendMessage(JSON.stringify({"type": "register", "client": this.identifier, "payload": {}}));
        }.bind(this));

        this.websocket.addOnCloseListener(function () {
            this.websocketReady = false;
        }.bind(this));

        this.websocket.addOnMessageListener(this.onWebsocketMessage.bind(this));
        this.websocket.open();
    }


    flushRemoteCandidates(id) {
        //Check if remote description exists already for that peer, otherwise would not be able to process candidate
        BABYLON.Tools.Log("RTC PC set remCand start" + id);
        var remDescExists = this.remoteDescriptions.hasOwnProperty(id);
        BABYLON.Tools.Log("RTC PC set remCand remDescExists" + remDescExists);
        var remDescNotEmpty = remDescExists && (this.remoteDescriptions[id].length > 0);
        BABYLON.Tools.Log("RTC PC set remCand remDescNotEmpty" + remDescNotEmpty);

        //BABYLON.Tools.Log("RTC remCand" + this.peerConnectionReady[id] + ":" + remDescNotEmpty + ":" + this.remoteDescriptions[id][0]["added"]);
        if (this.peerConnectionReady[id] && remDescNotEmpty && this.remoteDescriptions[id][0]["added"]) {
            BABYLON.Tools.Log("RTC PC set remCand allowed to add " + id);
            var rmCandidatesArray = this.remoteCandidates[id];
            for(var candNb in rmCandidatesArray) {
                var rmCand = rmCandidatesArray[candNb];
                if (!rmCand["added"]) {
                    var candidate = rmCand["payload"]["candidate"];
                    var mid= rmCand["payload"]["mid"];
                    BABYLON.Tools.Log("RTC PC set remCand" + id);
                    RTC.setRemoteCandidate(id, candidate, mid);
                    BABYLON.Tools.Log("RTC PC after set remCand" + id);
                    rmCand["added"] = true;
                }
            }
        }
    }

    flushRemoteDescriptions(id) {
        if (this.peerConnectionReady[id] && id in this.remoteDescriptions) {
            var rmDescriptionsArray = this.remoteDescriptions[id];
            for(var descNb in rmDescriptionsArray) {
                var rmDesc = rmDescriptionsArray[descNb];
                if (!rmDesc["added"]) {
                    var sdp = rmDesc["payload"]["sdp"];
                    var msgType = rmDesc["payload"]["type"];
                    BABYLON.Tools.Log("RTC PC set remDesc" + id);
                    RTC.setRemoteDescription(id, sdp, msgType);
                    BABYLON.Tools.Log("RTC PC after set remDesc" + id);
                    rmDesc["added"] = true;
                    this.remoteDescriptions[id][descNb]["added"] = true;

                }
            }
        }
    }

    appendRemoteData(appendTo, rmId, data) {
        if (!(rmId in appendTo)) appendTo[rmId] = [];
        appendTo[rmId].push(
            {
                "added": false,
                "payload": data,
            }
        )
    }


    onWebsocketMessage(message) {
        var msgData = JSON.parse(message);

        var clientName = msgData["client"];
        var msgType = msgData["type"];
        var msgPayload = msgData["payload"];
        if (clientName != this.identifier) {
            // We only pay attention to other client messages
            if(msgType == "register") {
                RTC.createDataChannel(clientName, clientName);
            }
            if (msgType == "offer") {
                // We are offered to start a connection with peer "clientName".
                RTC.startPeerConnection(clientName);
            }
            if (msgType == "offer" || msgType == "answer") {
                // We can set the description of remote peer "clientName".
                BABYLON.Tools.Log("RTC PC remoteDesc set! " + msgType);
                //RTC.setRemoteDescription(clientName, msgPayload["sdp"], msgType);
                this.appendRemoteData(this.remoteDescriptions, clientName, { "sdp": msgPayload["sdp"], "type": msgType });
                BABYLON.Tools.Log("RTC PC after set! " + msgType);
                this.flushRemoteDescriptions(clientName);
                BABYLON.Tools.Log("RTC PC after flush remDesc! " + msgType);
            }
            if (msgType == "candidate") {
                this.appendRemoteData(this.remoteCandidates, clientName,{ "candidate": msgPayload["candidate"], "mid": msgPayload["mid"]});
                this.flushRemoteCandidates(clientName);
                BABYLON.Tools.Log("RTC PC after flush remCand! " + msgType);
            }
        }
    }

    addOnDataChannelOpenListener(callback) {
        RTC.onDataChannel(function (dcData) {
            BABYLON.Tools.Log("RTC PC OPEN!");
            this.callback(dcData["channelId"]);
        }.bind({callback: callback}));
        //RTC.sendMessage(channelId, "Incredible! " + channelId + "? Do you copy? It's " + myName + ". Over!");
    };

    sendMessage(peerId, message) {
        if (peerId in this.dataChannels) RTC.sendMessage(peerId, message);
    }

    addOnMessageListener(callback) {
        RTC.onMessage(function (dcData) {
            var channelId = dcData["channelId"];
            var msg = dcData["message"];
            this.callback(channelId, msg);
        }.bind({callback: callback}));
    }
    
}

function str2ab(str) {
    var buf = new ArrayBuffer(str.length*2); // 2 bytes for each char
    var bufView = new Uint16Array(buf);
    for (var i=0, strLen=str.length; i < strLen; i++) {
    bufView[i] = str.charCodeAt(i);
    }
    return buf;
}

/*
function CreateBoxAsync(scene) {
    BABYLON.Tools.Log("In create box");
    const req = new XMLHttpRequest();
    req.responseType = "arraybuffer";
    BABYLON.Tools.Log("xhttp");
    req.addEventListener("loadend", () => {
        BABYLON.Tools.Log("eventlisten in resp");
        var resp = req.response;
        BABYLON.Tools.Log("resp" + req.status + "," + resp.byteLength); //+ "," + req.responseText + "," + req.responseType + "," + req.responseURL + "," + req.readyState);
        //var resp = str2ab(req.responseText);
        if(resp) {
            BABYLON.Tools.Log("resp not empty" + resp.byteLength);
            getRenderingCanvas.loadTTFAsync("Arial", resp).then(initAll);
            BABYLON.Tools.Log("after load ttf");
        }
    });
    BABYLON.Tools.Log("eventlisten after");
    req.open("GET", "app:///Arial.ttf");
    //"https://appassets.androidplatform.net/assets/Arial.ttf");//"app://assets/Arial.ttf");//https://github.com/matomo-org/travis-scripts/raw/master/fonts/Arial.ttf");//"https://appassets.androidplatform.net/assets/Arial.ttf");
    BABYLON.Tools.Log("eventlisten after open");
    req.send();
    BABYLON.Tools.Log("eventlisten after send");
}
*/
var engine = new BABYLON.NativeEngine();
var scene = new BABYLON.Scene(engine);
BABYLON.Tools.LoadFile("app:///droidsans.ttf", (data) => {
    BABYLON.Tools.Log("before font load");
    _native.Canvas.loadTTFAsync("droidsans", data).then(function () {
        BABYLON.Tools.Log("on font load");
        initAll();
        //_native.RootUrl = "https://playground.babylonjs.com";
        //console.log("Starting");
        //TestUtils.setTitle("Starting Native Validation Tests");
        //TestUtils.updateSize(testWidth, testHeight);
        //xhr.send();
    });
}, undefined, undefined, true);

function CreateSpheresAsync(scene) {
    var size = 12;
    for (var i = 0; i < size; i++) {
        for (var j = 0; j < size; j++) {
            for (var k = 0; k < size; k++) {
                var sphere = BABYLON.Mesh.CreateSphere("sphere" + i + j + k, 32, 0.9, scene);
                sphere.position.x = i;
                sphere.position.y = j;
                sphere.position.z = k;
            }
        }
    }

    return Promise.resolve();
}

BABYLON.Tools.Log("Before create box");
//CreateBoxAsync(scene);

var remoteDescriptions = {};
var remoteCandidates = {};

var initAll = function () {
    BABYLON.Mesh.CreateBox("box1", 0.2, scene);
    //CreateSpheresAsync(scene).then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/Box/glTF/Box.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/BoxTextured/glTF/BoxTextured.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/Suzanne/glTF/Suzanne.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/Avocado/glTF/Avocado.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/BoomBox/glTF/BoomBox.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/Sponza/glTF/Sponza.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/BrainStem/glTF/BrainStem.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/FlightHelmet/glTF/FlightHelmet.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/EnvironmentTest/glTF/EnvironmentTest.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/BoxAnimated/glTF/BoxAnimated.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/AnimatedMorphCube/glTF/AnimatedMorphCube.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/RiggedSimple/glTF/RiggedSimple.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/stevk/glTF-Asset-Generator/skins/Output/Animation_Skin/Animation_Skin_01.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/RiggedFigure/glTF/RiggedFigure.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/CesiumMan/glTF/CesiumMan.gltf").then(function () {
    //BABYLON.SceneLoader.AppendAsync("https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/ClearCoatTest/glTF/ClearCoatTest.gltf").then(function () {
    BABYLON.Tools.Log("Loaded");

    // This creates and positions a free camera (non-mesh)
    scene.createDefaultCamera(true, true, true);
    scene.activeCamera.alpha += Math.PI;

    var advancedTexture = BABYLON.GUI.AdvancedDynamicTexture.CreateFullscreenUI("UI");

    var button1 = BABYLON.GUI.Button.CreateSimpleButton("but1", "Click Me");
    button1.width = "150px"
    button1.height = "40px";
    button1.color = "white";
    button1.cornerRadius = 20;
    button1.background = "green";
    button1.onPointerUpObservable.add(function() {
        
        BABYLON.Mesh.CreateBox("box2", 0.5, scene);
    });
    advancedTexture.addControl(button1); 

    if (ibl) {
        scene.createDefaultEnvironment({ createGround: false, createSkybox: false });
    }
    else {
        scene.createDefaultLight(true);
    }

    if (cameraTexture) {
        scene.activeCamera.position.set(0, 1, -10);
        scene.activeCamera.setTarget(new BABYLON.Vector3(0, 1, 0));

        scene.meshes[0].setEnabled(false);
        var plane = BABYLON.MeshBuilder.CreatePlane("plane", { size: 1, sideOrientation: BABYLON.Mesh.DOUBLESIDE });
        plane.rotation.y = Math.PI;
        plane.rotation.z = Math.PI;

        plane.position.y = 1;

        var mat = new BABYLON.StandardMaterial("mat", scene);
        mat.diffuseColor = BABYLON.Color3.Black();

        var tex = BABYLON.VideoTexture.CreateFromWebCam(scene, function (videoTexture) {
            const videoSize = videoTexture.getSize();
            mat.emissiveTexture = videoTexture;
            plane.material = mat;
            plane.scaling.x = 5;
            plane.scaling.y = 5 * (videoSize.height / videoSize.width);
            console.log("Video texture size: " + videoSize);
        }, { maxWidth: 1280, maxHeight: 720, facingMode: 'environment' });
    }

    if (readPixels) {
        const texture = new BABYLON.Texture("https://assets.babylonjs.com/textures/earth.jpg", scene);
        texture.onLoadObservable.addOnce(() => {
            const mip = 1;
            const textureWidth = texture.getSize().width >> mip;
            const textureHeight = texture.getSize().height >> mip;
            const x = textureWidth / 4;
            const y = textureHeight / 4;
            const width = textureWidth / 2;
            const height = textureHeight / 2;
            // This read will create a new buffer.
            texture.readPixels(undefined, mip, undefined, undefined, undefined, x, y, width, height).then((buffer) => {
                console.log(`Read ${buffer.byteLength} pixel bytes.`);
                return buffer;
            })
                .then(buffer => {
                    // This read reuses the existing buffer.
                    texture.readPixels(undefined, mip, buffer, undefined, undefined, x, y, width, height).then((buffer) => {
                        console.log(`Read ${buffer.byteLength} pixel bytes.`);
                    });
                });
        });
    }

    if (wireframe) {
        var material = new BABYLON.StandardMaterial("wireframe", scene);
        material.wireframe = true;
        material.pointsCloud = true;

        for (var index = 0; index < scene.meshes.length; index++) {
            scene.meshes[0].material = material;
        }
    }

    if (rtt) {
        var rttTexture = new BABYLON.RenderTargetTexture("rtt", 1024, scene);
        scene.meshes.forEach(mesh => {
            rttTexture.renderList.push(mesh);
        });
        rttTexture.activeCamera = scene.activeCamera;
        rttTexture.vScale = -1;

        scene.customRenderTargets.push(rttTexture);

        var rttMaterial = new BABYLON.StandardMaterial("rttMaterial", scene);
        rttMaterial.diffuseTexture = rttTexture;

        var plane = BABYLON.MeshBuilder.CreatePlane("rttPlane", { width: 4, height: 4 }, scene);
        plane.position.y = 1;
        plane.position.z = -5;
        plane.rotation.y = Math.PI;
        plane.material = rttMaterial;
    }

    if (turntable) {
        scene.beforeRender = function () {
            scene.meshes[0].rotate(BABYLON.Vector3.Up(), 0.005 * scene.getAnimationRatio());
        };
    }

    if (logfps) {
        var wsOpened = false;
        var rtcInstance = null;
        var wsStarted = false;
        const WS_URL = "ws://90.16.68.220:25560";
        const myName = "Merryviaphone";

        const wsConnection = new RTCWebsocket(WS_URL);
        const rtcConnection = new RTCConnection(myName, wsConnection);

        rtcConnection.addOnDataChannelOpenListener(function (channelId) {
            BABYLON.Tools.Log("RTC DC Open! " + channelId);
            this.rtcConnection.sendMessage(channelId, "Hello! " + channelId);
        }.bind({rtcConnection: rtcConnection}));

        rtcConnection.addOnMessageListener((channelId, msg) => {
            BABYLON.Tools.Log("RTC DC MSG! " + channelId + ": " + msg);
            
        })


        var logFpsLoop = () => {
            if (window.navigator.rtc) {
                BABYLON.Tools.Log("RTC ok");
                rtcInstance = window.navigator.rtc;
                //if (window.navigator.rtc.openWebsocket) {
                //TODO: Events bellow must be registered only once !
                if (!wsStarted) {
                    /*
                    RTC.onPeerConnectionCreated((pcData) => {
                        var id = pcData["id"];
                        BABYLON.Tools.Log("RTC PC on create! " + id);

                        if (id in remoteDescriptions) {
                            var sdp = remoteDescriptions[id]["sdp"];
                            var msgType = remoteDescriptions[id]["type"];
                            // id == clientName
                            BABYLON.Tools.Log("RTC PC set remDesc" + id);
                            RTC.setRemoteDescription(id, sdp, msgType);
                        }

                    })
                    RTC.onDataChannel((dcData) => {
                        BABYLON.Tools.Log("RTC On data channel");
                        var channelId = dcData["channelId"];
                        BABYLON.Tools.Log("RTC MSG " + channelId + " is here!");
                        RTC.sendMessage(channelId, "Incredible! " + channelId + "? Do you copy? It's " + myName + ". Over!");
                    });
                    BABYLON.Tools.Log("RTC datachannel ok");
                    RTC.onMessage((dcData) => {
                        var channelId = dcData["channelId"];
                        var msg = dcData["message"];
                        BABYLON.Tools.Log("RTC MSG " + channelId + " said " + msg);
                    });
                    BABYLON.Tools.Log("RTC message ok");
                    RTC.onLocalDescription((descData) => {
                        BABYLON.Tools.Log("RTC On local description");
                        var type = descData["type"];
                        var sdp = descData["sdp"];

                        //We have to send it over websocket to connect to peer
                        if (wsOpened) {
                            var msgToSend = {
                                "client": myName,
                                "type": type,
                                "payload": {
                                    "sdp": sdp,
                                }
                            };
                            RTC.sendWebsocketMessage(WS_URL, JSON.stringify(msgToSend));
                        }
                    });
                    BABYLON.Tools.Log("RTC local desc ok");
                    RTC.onLocalCandidate((candidateData) => {
                        BABYLON.Tools.Log("RTC On local candidate");
                        var candidate = candidateData["candidate"];
                        var mid = candidateData["mid"];

                        //We have to send it over websocket to connect to peer
                        if (wsOpened) {
                            var msgToSend = {
                                "client": myName,
                                "type": "candiate",
                                "payload": {
                                    "candidate": candidate,
                                    "mid": mid,
                                }
                            };
                            RTC.sendWebsocketMessage(WS_URL, JSON.stringify(msgToSend));
                        }

                    });
                    BABYLON.Tools.Log("RTC local cand ok");

                    RTC.onWebsocketOpen((wsData) => {
                        wsOpened = true;
                        var wsId = wsData["wsUrl"];
                        BABYLON.Tools.Log("RTC WS OPENED! " + wsId);
                        //START RTC HERE
                    });
                    BABYLON.Tools.Log("RTC ws open ok");
                    RTC.onWebsocketClosed((wsData) => {
                        var wsId = wsData["wsUrl"];
                        BABYLON.Tools.Log("RTC WS CLOSED! " + wsId);
                        wsOpened = false;
                    });
                    BABYLON.Tools.Log("RTC ws closed ok");
                    RTC.onWebsocketMessage((wsData) => {

                        var wsId = wsData["wsUrl"];
                        var msg = wsData["message"];
                        BABYLON.Tools.Log("RTC WS MSG! " + msg);

                        var msgData = JSON.parse(msg);
                        //RTC forwarding logic

                        var clientName = msgData["client"];
                        var msgType = msgData["type"];
                        var msgPayload = msgData["payload"];
                        if (clientName != myName) {
                            // We only pay attention to other client messages
                            if (msgType == "offer") {
                                // We are offered to start a connection with peer "clientName".
                                // Perhaps start pc must async...
                                RTC.startPeerConnection(clientName);
                            }
                            if (msgType == "offer" || msgType == "answer") {
                                // We can set the description of remote peer "clientName".
                                BABYLON.Tools.Log("RTC PC remoteDesc set! " + msgType);
                                //RTC.setRemoteDescription(clientName, msgPayload["sdp"], msgType);
                                remoteDescriptions[clientName] = { "sdp": msgPayload["sdp"], "type": msgType };
                                BABYLON.Tools.Log("RTC PC after set! " + msgType);
                            }
                            if (msgType == "candidate") {

                                //TODO: do this on callback of peer connection created
                                // or if created...
                                RTC.setRemoteCandidate(clientName, msgPayload["candidate"], msgPayload["mid"]);
                            }
                        }
                        //BABYLON.Tools.Log("RTC WS RECV! - " + wsId + ": " + msg);
                        //rtcInstance.sendWebsocketMessage(wsId, "Hi! Did you get it?");

                    });
                    BABYLON.Tools.Log("RTC ws msg");
                    RTC.openWebsocket(WS_URL);
                    */
                    wsStarted = true;
                }
                /*
                if (rtcInstance && window.navigator.rtc.openWebsocket) {
                    BABYLON.Tools.Log("RTC method ok");
                    if (!wsOpened) {
                        rtcInstance.onWebsocketOpen((wsData) => {
                            var wsId = wsData["wsUrl"];
                            BABYLON.Tools.Log("RTC WS OPENED! " + wsId);
                            rtcInstance.sendWebsocketMessage(wsId, "Hello from native websocket!");
                        });
                        rtcInstance.onWebsocketMessage((wsData) => {
                            var wsId = wsData["wsUrl"];
                            var msg = wsData["message"];
                            BABYLON.Tools.Log("RTC WS RECV! - " + wsId + ": " + msg);
                            rtcInstance.sendWebsocketMessage(wsId, "Hi! Did you get it?");
                        });
                        BABYLON.Tools.Log("RTC callback reg ");
                        var wsId = rtcInstance.openWebsocket("ws://90.16.115.90:25560");
                        BABYLON.Tools.Log("RTC started WS! " + wsId);
                    }
                }
                */
            }
            if (window.navigator.xr.GEO_getEarthQuaternionLatitudeLongitude) {
                //var retEarth = window.navigator.earth.GEO_getEarthQuaternionLatitudeLongitude();
                //BABYLON.Tools.Log("Earth ok2");
                //BABYLON.Tools.Log("EarthObj " + retEarth.lat + " " + retEarth.lon);
                BABYLON.Tools.Log("Try Earth ok2");
                var retEarth = window.navigator.xr.GEO_getEarthQuaternionLatitudeLongitude();
                BABYLON.Tools.Log("EarthObj " + retEarth.lat + " " + retEarth.lon);
                BABYLON.Tools.Log("xr geo");
            }
            if (window.navigator.xr.native.GEO_getEarthQuaternionLatitudeLongitude) {
                //var retEarth = window.navigator.earth.GEO_getEarthQuaternionLatitudeLongitude();
                //BABYLON.Tools.Log("Earth ok2");
                //BABYLON.Tools.Log("EarthObj " + retEarth.lat + " " + retEarth.lon);
                BABYLON.Tools.Log("xr geo native");
            }
            if (window.navigator && window.navigator.xr) {
                BABYLON.Tools.Log("XR OK");
                if (window.navigator.xr.native) {
                    //window.navigator.xr.native.requestSession();
                    BABYLON.Tools.Log("NativeXR OK");
                }
            }
            BABYLON.Tools.Log("FPS: " + Math.round(engine.getFps()));
            window.setTimeout(logFpsLoop, 1000);
        };

        window.setTimeout(logFpsLoop, 3000);
    }

    engine.runRenderLoop(function () {
        scene.render();
    });

    if (vr || ar || hololens) {
        setTimeout(function () {
            scene.createDefaultXRExperienceAsync({ disableDefaultUI: true, disableTeleportation: true }).then((xr) => {
                if (xrHitTest) {
                    // Create the hit test module. OffsetRay specifies the target direction, and entityTypes can be any combination of "mesh", "plane", and "point".
                    const xrHitTestModule = xr.baseExperience.featuresManager.enableFeature(
                        BABYLON.WebXRFeatureName.HIT_TEST,
                        "latest",
                        { offsetRay: { origin: { x: 0, y: 0, z: 0 }, direction: { x: 0, y: 0, z: -1 } }, entityTypes: ["mesh"] });

                    // When we receive hit test results, if there were any valid hits move the position of the mesh to the hit test point.
                    xrHitTestModule.onHitTestResultObservable.add((results) => {
                        if (results.length) {
                            scene.meshes[0].position.x = results[0].position.x;
                            scene.meshes[0].position.y = results[0].position.y;
                            scene.meshes[0].position.z = results[0].position.z;
                        }
                    });
                }
                else {
                    setTimeout(function () {
                        scene.meshes[0].position.z = 2;
                        scene.meshes[0].rotate(BABYLON.Vector3.Up(), 3.14159);
                    }, 5000);
                }

                // Showing visualization for ARKit LiDAR mesh data
                if (meshDetection) {
                    var mat = new BABYLON.StandardMaterial("mat", scene);
                    mat.wireframe = true;
                    mat.diffuseColor = BABYLON.Color3.Blue();
                    const xrMeshes = xr.baseExperience.featuresManager.enableFeature(
                        BABYLON.WebXRFeatureName.MESH_DETECTION,
                        "latest",
                        { convertCoordinateSystems: true });
                    console.log("Enabled mesh detection.");
                    const meshMap = new Map();

                    // adding meshes
                    xrMeshes.onMeshAddedObservable.add(mesh => {
                        try {
                            console.log("Mesh added.");
                            // create new mesh object
                            var customMesh = new BABYLON.Mesh("custom", scene);
                            var vertexData = new BABYLON.VertexData();
                            vertexData.positions = mesh.positions;
                            vertexData.indices = mesh.indices;
                            vertexData.normals = mesh.normals;
                            vertexData.applyToMesh(customMesh, true);
                            customMesh.material = mat;
                            // add mesh and mesh id to map
                            meshMap.set(mesh.id, customMesh);
                        } catch (ex) {
                            console.error(ex);
                        }
                    });

                    // updating meshes
                    xrMeshes.onMeshUpdatedObservable.add(mesh => {
                        try {
                            console.log("Mesh updated.");
                            if (meshMap.has(mesh.id)) {
                                var vertexData = new BABYLON.VertexData();
                                vertexData.positions = mesh.positions;
                                vertexData.indices = mesh.indices;
                                vertexData.normals = mesh.normals;
                                vertexData.applyToMesh(meshMap.get(mesh.id), true);
                            }
                        } catch (ex) {
                            console.error(ex);
                        }
                    });

                    // removing meshes
                    xrMeshes.onMeshRemovedObservable.add(mesh => {
                        try {
                            console.log("Mesh removed.");
                            if (meshMap.has(mesh.id)) {
                                meshMap.get(mesh.id).dispose();
                                meshMap.delete(mesh.id);
                            }
                        } catch (ex) {
                            console.error(ex);
                        }
                    });
                }

                // Below is an example of how to process feature points.
                if (xrFeaturePoints) {
                    // First we attach the feature point system feature the XR experience.
                    const xrFeaturePointsModule = xr.baseExperience.featuresManager.enableFeature(
                        BABYLON.WebXRFeatureName.FEATURE_POINTS,
                        "latest",
                        {});

                    // Next We create the point cloud system which we will use to display feature points.
                    var pcs = new BABYLON.PointsCloudSystem("pcs", 5, scene);
                    var featurePointInitFunc = function (particle, i, s) {
                        particle.position = new BABYLON.Vector3(0, -5, 0);
                    }

                    // Add some starting points and build the mesh.
                    pcs.addPoints(3000, featurePointInitFunc);
                    pcs.buildMeshAsync().then((mesh) => {
                        mesh.alwaysSelectAsActiveMesh = true;
                    });

                    // Define the logic for how to display a particular particle in the particle system
                    // which represents a feature point.
                    pcs.updateParticle = function (particle) {
                        // Grab the feature point cloud from the xrFeaturePointsModule.
                        var featurePointCloud = xrFeaturePointsModule.featurePointCloud;

                        // Find the index of this particle in the particle system. If there exists a
                        // mapping to a feature point then display its position, otherwise hide it somewhere far away.
                        var index = particle.idx;
                        if (index >= featurePointCloud.length) {
                            // Hide the particle not currently in use.
                            particle.position = new BABYLON.Vector3(-100, -100, -100);
                        }
                        else {
                            // To display a feature point set its position to the position of the feature point
                            // and set its color on the scale from white->red where white = least confident and
                            // red = most confident.
                            particle.position = featurePointCloud[index].position;
                            particle.color = new BABYLON.Color4(1, 1 - featurePointCloud[index].confidenceValue, 1 - featurePointCloud[index].confidenceValue, 1);
                        }

                        return particle;
                    }

                    // Listen for changes in feature points both being added and updated, and only update
                    // our display every 60 changes to the feature point cloud to avoid slowdowns.
                    var featurePointChangeCounter = 0;
                    xrFeaturePointsModule.onFeaturePointsAddedObservable.add((addedPointIds) => {
                        if (++featurePointChangeCounter % 60 == 0) {
                            pcs.setParticles();
                        }
                    });

                    xrFeaturePointsModule.onFeaturePointsUpdatedObservable.add((updatedPointIds) => {
                        if (++featurePointChangeCounter % 60 == 0) {
                            pcs.setParticles();
                        }
                    });
                }

                let sessionMode = vr ? "immersive-vr" : "immersive-ar"
                if (hololens) {
                    // Because HoloLens 2 is a head mounted display, its Babylon.js immersive experience more closely aligns to vr
                    sessionMode = "immersive-vr";

                    // Below is an example for enabling hand tracking. The code is not unique to HoloLens 2, and may be reused for other WebXR hand tracking enabled devices.
                    xr.baseExperience.featuresManager.enableFeature(
                        BABYLON.WebXRFeatureName.HAND_TRACKING,
                        "latest",
                        { xrInput: xr.input });
                }

                // Test image tracking and detection.
                // To test image tracking locally either bring up the images below on your machine by loading the URL or by printing them out.
                // Then gain tracking on them during the AR Session by orienting your camera towards the image, tracking will be represented by a colored cube at the center of the image.
                if (imageTracking) {
                    const webXRTrackingMeshes = [];
                    const webXRImageTrackingModule = xr.baseExperience.featuresManager.enableFeature(
                        BABYLON.WebXRFeatureName.IMAGE_TRACKING,
                        "latest",
                        {
                            images: [
                                { src: "https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/IridescentDishWithOlives/screenshot/screenshot_Large.jpg", estimatedRealWorldWidth: .2 },
                                { src: "https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/DragonAttenuation/screenshot/screenshot_large.png", estimatedRealWorldWidth: .2 },
                            ]
                        });

                    webXRImageTrackingModule.onTrackedImageUpdatedObservable.add((imageObject) => {
                        if (webXRTrackingMeshes[imageObject.id] === undefined) {
                            webXRTrackingMeshes[imageObject.id] = BABYLON.Mesh.CreateBox("box1", 0.05, scene);
                            const mat = new BABYLON.StandardMaterial("mat", scene);
                            mat.diffuseColor = BABYLON.Color3.Random();
                            webXRTrackingMeshes[imageObject.id].material = mat;
                        }
                        webXRTrackingMeshes[imageObject.id].setEnabled(!imageObject.emulated);
                        imageObject.transformationMatrix.decomposeToTransformNode(webXRTrackingMeshes[imageObject.id]);
                    });
                }

                xr.baseExperience.enterXRAsync(sessionMode, "unbounded", xr.renderTarget).then((xrSessionManager) => {
                    if (hololens) {
                        // Pass through, head mounted displays (HoloLens 2) require autoClear and a black clear color
                        xrSessionManager.scene.autoClear = true;
                        xrSessionManager.scene.clearColor = new BABYLON.Color4(0, 0, 0, 0);
                    }
                });
            });
        }, 5000);
    }

    if (text) {
        var Writer = BABYLON.MeshWriter(scene, { scale: 1.0, defaultFont: "Arial" });
        new Writer(
            "Lorem ipsum dolor sit amet...",
            {
                "anchor": "center",
                "letter-height": 5,
                "color": "#FF0000"
            }
        );
    }

}
