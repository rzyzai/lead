let data_list = [];
let word = "";
let index = 0;
let prompted = false;

function prev_quiz() {
    if (data_list.length == 1) {
        mdui.snackbar("没有上一个了");
    } else {
        data_list.pop();
        word = data_list[data_list.length - 1]["word"];
        index = data_list[data_list.length - 1]["index"];
        apply_quiz(data_list[data_list.length - 1]["quiz"]);
    }
}

function apply_quiz(new_quiz) {
    $("#A").attr("class", "mdui-list-item", "mdui-ripple");
    $("#B").attr("class", "mdui-list-item", "mdui-ripple");
    $("#C").attr("class", "mdui-list-item", "mdui-ripple");
    $("#D").attr("class", "mdui-list-item", "mdui-ripple");
    $("#explanation").html("");
    $("#question").html(new_quiz["question"]);
    $("#A").html("A. " + new_quiz["options"]["A"]);
    $("#B").html("B. " + new_quiz["options"]["B"]);
    $("#C").html("C. " + new_quiz["options"]["C"]);
    $("#D").html("D. " + new_quiz["options"]["D"]);
    prompted = false;
}

function get_quiz(word_index = -1) {
    $.ajax({
        type: 'GET',
        url: "api/get_quiz",
        data:
            (word_index == -1) ? ({}) : ({word_index: word_index}),
        success: function (result) {
            data_list.push(result)
            word = result["word"]
            index = result["index"]
            apply_quiz(data_list[data_list.length - 1]["quiz"])
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function quiz_select(opt) {
    if (opt == data_list[data_list.length - 1]["quiz"]["answer"]) {
        if (!prompted) {
            $.ajax({
                type: 'GET',
                url: "api/quiz_passed",
                data:
                    {
                        word_index: index,
                    },
                error: function (XMLHttpRequest, textStatus, errorThrown) {
                    console.log(XMLHttpRequest.status);
                    console.log(XMLHttpRequest.readyState);
                    console.log(textStatus);
                }
            });
        }
        get_quiz()
        apply_quiz(data_list[data_list.length - 1]["quiz"])
    } else {
        if (!prompted) {
            $.ajax({
                type: 'GET',
                url: "api/quiz_failed",
                data:
                    {
                        word_index: index,
                    },
                error: function (XMLHttpRequest, textStatus, errorThrown) {
                    console.log(XMLHttpRequest.status);
                    console.log(XMLHttpRequest.readyState);
                    console.log(textStatus);
                }
            });
        }
        $("#" + opt).addClass("mdui-color-red");
    }
}

function get_explanation_panel(title, summary, body)
{
    return '<div class="mdui-panel-item mdui-panel-item-open">' +
        '<div class="mdui-panel-item-header">' +
        '<div class="mdui-panel-item-title">' + title + '</div>' +
        '<div class="mdui-panel-item-summary">' + summary + '</div></div>' +
        '<div class="mdui-panel-item-body">' + body + '</div></div>';
}

function prompt_explanation_panel(result, opt)
{
    return get_explanation_panel(opt, data_list[data_list.length - 1]["quiz"]["options"][opt], result[opt]);
}

function quiz_prompt(opt) {
    if (!prompted) {
        prompted = true;
        $("#" + opt).addClass("mdui-color-red");
        $.ajax({
            type: 'GET',
            url: "api/quiz_prompt",
            data:
                {
                    A_index: data_list[data_list.length - 1]["quiz"]["indexes"]["A"],
                    B_index: data_list[data_list.length - 1]["quiz"]["indexes"]["B"],
                    C_index: data_list[data_list.length - 1]["quiz"]["indexes"]["C"],
                    D_index: data_list[data_list.length - 1]["quiz"]["indexes"]["D"],
                    word_index: index,
                },
            success: function (result) {
                if (result["status"] == "success") {
                    $("#explanation").html('<div class="mdui-panel" mdui-panel>' +
                        prompt_explanation_panel(result, "A") +
                        prompt_explanation_panel(result, "B") +
                        prompt_explanation_panel(result, "C") +
                        prompt_explanation_panel(result, "D") +
                        '</div>');
                }
            },
            error: function (XMLHttpRequest, textStatus, errorThrown) {
                console.log(XMLHttpRequest.status);
                console.log(XMLHttpRequest.readyState);
                console.log(textStatus);
            }
        });
        $("#" + data_list[data_list.length - 1]["quiz"]["answer"]).addClass("mdui-color-green");
    }
}

function pass() {
    $.ajax({
        type: 'GET',
        url: "api/pass",
        data:
            {
                word_index: index,
            },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
    get_quiz()
}

function get_progress()
{
    $.ajax({
        type: 'GET',
        url: "api/get_progress",
        success: function (result) {
            if (result["status"] == "success") {
                $("#passed_word_count").html(result["passed_word_count"]);
                $("#word_count").html(result["word_count"]);
                $("#progress_bar").attr('style', 'width:' + (result["passed_word_count"] / result["word_count"]) * 100 + '%;')
            }
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}