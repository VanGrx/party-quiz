let answerGiven = false;
let newQuestion = false;
let currentQuestion = 0;

let ws = null;

function giveAnswer(index) {


    let data = {};

    data["answer"] = parseInt(index);
    data["playerID"] = document.getElementById("playerID").innerHTML;

    answerGiven = true;

    if (ws) {

        data["type"] = "player";
        data["method"] = "giveAnswer";

        ws.send(JSON.stringify(data));
        handlePlayerAnswerGiven();

    } else {

        $.ajax({
            type: "POST",
            url: "./player.html",
            data: data,
            success: function (data) {
                handlePlayerAnswerGiven();

            },
            error: function (error) {
                console.log("Error:");
                handlePlayerAnswerGiven();
            }
        });
    }
}


function handlePlayerAnswerGiven() {
    document.getElementById("pitanjeDiv").style.display = "none";

}


function handlePlayerStatus(status) {


    document.getElementById("playerID").innerHTML = status["player"]["id"];
    document.getElementById("playerUsername").innerHTML = status["player"]["username"];
    document.getElementById("score").innerHTML = status["player"]["score"];


    let currQuestion = status["currQuestion"] + 1;
    document.getElementById("questions").innerHTML = currQuestion + "/" + status["totalQuestions"];


    let gameStatus = status["gameState"]

    if (gameStatus !== 2) {


    } else if (gameStatus === 2 && currQuestion !== currentQuestion) {

        currentQuestion = currQuestion;
        answerGiven = false;
        document.getElementById("pitanjeDiv").style.display = "block";
        document.getElementById("question").innerHTML = status["question"];

        for (let i = 1; i <= 4; i++)
            document.getElementById("answer" + i).innerHTML = status["answers"][i - 1];

    } else if (answerGiven === true) {
        document.getElementById("pitanjeDiv").style.display = "none";

    }
}


function getPlayerStatus() {

    let data = {};

    data["status"] = "1";

    $.ajax({
        type: "GET",
        url: "./player.html",
        data: data,
        success: function (data) {
            handlePlayerStatus(data);

        },
        error: function (error) {
            console.log("Error:");
            console.log(error);
        }
    });

}

function loginGame() {

    let data = {};
    data.roomNumber = parseInt(document.getElementById("roomNumber").value);
    data.username = document.getElementById("username").value;


    if ("WebSocket" in window) {
        ws = new WebSocket("ws://192.168.1.2:8080/webSocket");

        ws.onopen = function () {
            data.type = "player";
            data.method = "gameInit";
            ws.send(JSON.stringify(data));
            document.getElementById("myform").style.display = "none";
            document.getElementById("quizDiv").style.display = "block";
        };

        ws.onmessage = function (evt) {

            let received_msg = JSON.parse(evt.data);

            if (received_msg.player)
                handlePlayerStatus(received_msg);

        };

        ws.onclose = function () {

            // websocket is closed.
            alert("Connection is closed...");
            ws = null;
        };
    } else {
        $.ajax({
            type: "POST",
            url: "./player.html",
            data: data,
            success: function (data) {

                let sessionID = data["sessionID"];

                document.cookie = "sessionID=" + sessionID;

                document.getElementById("myform").style.show = "none";
                document.getElementById("quizDiv").style.display = "show";

                window.setInterval(function () {
                    getPlayerStatus();  //calling every .5 seconds
                }, 200);
            },
            error: function (error) {
                console.log("Error:");
                console.log(error);
            }
        });
    }

}

window.onbeforeunload = function () {
    alert("closing");
    if (ws.readyState === WebSocket.OPEN)
        ws.close();
};