var quiz;
var word;
var pronunciation;
var pos;
function get_quiz()
{
    $.ajax({
        type: 'GET',
        url: "get_quiz",
        success: function (result)
        {
            quiz = result["quiz"]
            word = result["word"]
            pronunciation = result["pronunciation"]
            pos = result["pos"]
            $("#word").html(word);
            $("#pronunciation").html(pronunciation);
            $("#A").html(quiz["options"]["A"]);
            $("#B").html(quiz["options"]["B"]);
            $("#C").html(quiz["options"]["C"]);
            $("#D").html(quiz["options"]["D"]);
        },
        error: function (XMLHttpRequest, textStatus, errorThrown)
        {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function quiz_select(opt)
{
    if(opt == quiz["answer"])
    {
        $.ajax({
            type: 'GET',
            url: "quiz_passed?pos=" + pos,
            error: function (XMLHttpRequest, textStatus, errorThrown)
            {
                console.log(XMLHttpRequest.status);
                console.log(XMLHttpRequest.readyState);
                console.log(textStatus);
            }
        });
        get_quiz()
    }
    else
    {
        $.ajax({
            type: 'GET',
            url: "quiz_failed?pos=" + pos,
            error: function (XMLHttpRequest, textStatus, errorThrown)
            {
                console.log(XMLHttpRequest.status);
                console.log(XMLHttpRequest.readyState);
                console.log(textStatus);
            }
        });
        alert("failed")
    }
}

function pass()
{
    $.ajax({
        type: 'GET',
        url: "pass?pos=" + pos,
        error: function (XMLHttpRequest, textStatus, errorThrown)
        {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
    get_quiz()
}

window.onload = get_quiz;
