var socket = io("https://esp32-iot-template.herokuapp.com");

$(function() {
    $("#btnOn").on("click", function() {
        socket.emit("client-send-data", {name: "Browser", address: "led", value: "on"});
    });
    $("#btnOff").on("click", function() {
        socket.emit("client-send-data", {name: "Browser", address: "led", value: "off"});
    })
});