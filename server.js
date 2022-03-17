var express = require("express");
var path = require("path")
var app = express();
app.use(express.static("public"));
app.use(express.static(__dirname + "/node_modules"));
app.set("view engine", "ejs");
app.set('views', path.join(__dirname, "views"));

var server = require("http").createServer(app);
var io = require("socket.io")(server);

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log("Server is up and running on port: " + PORT);
});

app.get("/", function(req, res, next) {
    res.redirect("home");
});
app.get("/home", function(req, res){
    res.render("home");
});

io.on("connection", function(socket){
    console.log("Client connected..");
    socket.on("browser-send-data", function(data){
        console.log(data);
        socket.broadcast.emit("server-send-data", data);
    });

    socket.on("client-send-data", function(data){
        console.log(data);
        socket.broadcast.emit("server-send-browser", data);
    });
});