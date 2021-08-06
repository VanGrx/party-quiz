

var answerGiven = false;
var newQuestion = false;
var currentQuestion = 0;

function giveAnswer(index){


var data = {};

data["answer"]=index;
data["playerID"]=document.getElementById("playerID").innerHTML;

answerGiven = true;

$.ajax({
  type: "POST",
  url: "./player.html",
  data: data,
  success: function(data){
  document.getElementById("pitanjeDiv").style.display = "none";

      },
  error: function(error){
    console.log("Error:");
  document.getElementById("pitanjeDiv").style.display = "none";
}
});

}


function handlePlayerStatus(status){


document.getElementById("playerID").innerHTML = status["player"]["id"];
document.getElementById("playerUsername").innerHTML = status["player"]["username"];

document.getElementById("score").innerHTML = status["player"]["score"];


var currQuestion = status["currQuestion"]+1;
var questionText = currQuestion+"/"+status["totalQuestions"];
document.getElementById("questions").innerHTML = questionText;


var gameStatus = status["gameState"]

if(status["gameState"] != 2){


} else if(status["gameState"] == 2 && currQuestion!=currentQuestion){

  currentQuestion = currQuestion;
  answerGiven = false;
  document.getElementById("pitanjeDiv").style.display = "block";
  document.getElementById("question").innerHTML = status["question"];
  document.getElementById("answer1").innerHTML = status["answers"][0];
  document.getElementById("answer2").innerHTML = status["answers"][1];
  document.getElementById("answer3").innerHTML = status["answers"][2];
  document.getElementById("answer4").innerHTML = status["answers"][3];



} else if(answerGiven == true){
document.getElementById("pitanjeDiv").style.display = "none";

}
}


function getPlayerStatus(){

var data = {};

data["status"]="1";

var res = JSON.stringify(data)

console.log(res);


$.ajax({
  type: "GET",
  url: "./player.html",
  data: data,
  success: function(data){
        handlePlayerStatus(data);

      },
  error: function(error){
    console.log("Error:");
    console.log(error);
}
});

}

window.setInterval(function(){
  getPlayerStatus();  //calling every .5 seconds
}, 200);