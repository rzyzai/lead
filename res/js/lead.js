"use strict"
let quiz_data_list = [];
let quiz_word = "";
let quiz_word_index = 0;
let quiz_prompt_data = null;
let quiz_prompted = false;
let quiz_prompt_panel_isopen = [false, false, false, false];

let memorize_word = "";
let memorize_index = 0;
let memorize_meaning = "";

let search_data = null;

function init_home()
{
    $.ajax({
        type: 'GET',
        url: "api/get_plan",
        success: function (result) {
            if(result["status"] == "success") {
                if (result["finished_word_count"] == result["planned_word_count"]) {
                    $("#message").html("<h3>今日计划已完成</h3>");
                } else {
                    $("#finished_word_count").html(result["finished_word_count"]);
                    $("#planned_word_count").html(result["planned_word_count"]);
                }
                $("#progress_bar").attr('style', 'width:' + (result["finished_word_count"] / result["planned_word_count"]) * 100 + '%;');
            }
            else
            {
                mdui.snackbar(result["message"]);
            }
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}
function init_marked()
{
    $.ajax({
        type: 'GET',
        url: "api/get_marked",
        success: function (result) {
            if (result["status"] == "success") {
                var content = '<div class="mdui-panel" mdui-panel>';
                for (let i = result["marked_words"].length - 1; i >= 0;i--) {
                    content += marked_explanation_panel(result["marked_words"][i]);
                }
                content += '</div>'
                $("#marked-words").html(content);
                mdui.mutation();
            }
            else
            {
                mdui.snackbar(result["message"]);
            }
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function init_passed()
{
    $.ajax({
        type: 'GET',
        url: "api/get_passed",
        success: function (result) {
            if (result["status"] == "success") {
                $("#passed_word_count").html(result["passed_word_count"]);
                $("#word_count").html(result["word_count"]);
                $("#progress_bar").attr('style', 'width:' + (result["passed_word_count"] / result["word_count"]) * 100 + '%;')

                var content = '<div class="mdui-panel" mdui-panel>';
                for (let i = result["passed_words"].length - 1; i >= 0;i--) {
                    content += passed_explanation_panel(result["passed_words"][i]);
                }
                content += '</div>'
                $("#passed-words").html(content);
                mdui.mutation();
            }
            else
            {
                mdui.snackbar(result["message"]);
            }
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function init_search_result() {
    if(search_data == null)
        search_data = JSON.parse(window.sessionStorage.getItem("search_result"));
    else
        window.sessionStorage.setItem("search_result", search_data);
    var content = '<div class="mdui-panel" mdui-panel>';
    for (var word in search_data["words"])
        content += search_explanation_panel(search_data["words"], word);
    content += '</div>'
    $("#search-result").html(content);
    $("#search-title").html(search_data["message"]);
}

function init_prompt() {
    $("#explanation").html('<div class="mdui-panel" mdui-panel>' +
        prompt_explanation_panel(quiz_prompt_data, "A", quiz_prompt_panel_isopen[0]) +
        prompt_explanation_panel(quiz_prompt_data, "B", quiz_prompt_panel_isopen[1]) +
        prompt_explanation_panel(quiz_prompt_data, "C", quiz_prompt_panel_isopen[2]) +
        prompt_explanation_panel(quiz_prompt_data, "D", quiz_prompt_panel_isopen[3]) +
        '</div>');
    mdui.mutation();
}

function save_quiz_prompt_panel_status()
{
    var panels = document.getElementsByClassName("mdui-panel-item");
    quiz_prompt_panel_isopen[0] = panels[0].classList.length > 1;
    quiz_prompt_panel_isopen[1] = panels[1].classList.length > 1;
    quiz_prompt_panel_isopen[2] = panels[2].classList.length > 1;
    quiz_prompt_panel_isopen[3] = panels[3].classList.length > 1;
}

function update_memorize_data(result)
{
    memorize_word = result["word"];
    memorize_index = result["word_index"];
    memorize_meaning = result["meaning"];
    var content = '<div class="mdui-panel" mdui-panel>'
        + get_explanation_panel(memorize_word, memorize_meaning, result["content"]) + '</div>'
    $("#explanation").html(content);
}

function prev_word(){
    $.ajax({
        type: 'GET',
        url: "api/prev_memorize_word",
        success: function (result) {
            if(result["status"] == "success")
                update_memorize_data(result);
            else
                mdui.snackbar(result["message"]);
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function next_word(next) {
    next = typeof next !== 'undefined' ? next : false;
    $.ajax({
        type: 'GET',
        url: "api/memorize_word",
        data:
            {
                next: next
            },
        success: function (result) {
            if (result["status"] == "success")
                update_memorize_data(result);
            else
                mdui.snackbar(result["message"]);
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function mark_word(word_index)
{
    $.ajax({
        type: 'GET',
        data:
            {
                word_index: word_index,
            },
        url: "api/mark_word",
        success: function (result) {
            if(result["status"] == "success")
                mdui.snackbar("收藏成功");
            else
                mdui.snackbar(result["message"]);
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function unmark_word(word_index)
{
    $.ajax({
        type: 'GET',
        data:
            {
                word_index: word_index,
            },
        url: "api/unmark_word",
        success: function (result) {
            if(result["status"] == "success")
                mdui.snackbar("取消收藏成功");
            else
                mdui.snackbar(result["message"]);
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function prev_quiz() {
    if (quiz_data_list.length == 1) {
        mdui.snackbar("没有上一个了");
    } else {
        quiz_data_list.pop();
        quiz_word = quiz_data_list[quiz_data_list.length - 1]["quiz_word"];
        quiz_word_index = quiz_data_list[quiz_data_list.length - 1]["word_index"];
        apply_quiz(quiz_data_list[quiz_data_list.length - 1]["quiz"]);
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
    quiz_prompted = false;
}

function next_quiz(word_index) {
    word_index = typeof word_index !== 'undefined' ?  word_index : -1;
    $.ajax({
        type: 'GET',
        url: "api/get_quiz",
        data:
            (word_index == -1) ? ({}) : ({word_index: word_index}),
        success: function (result) {
            quiz_data_list.push(result)
            quiz_word = result["quiz_word"]
            quiz_word_index = result["word_index"]
            apply_quiz(quiz_data_list[quiz_data_list.length - 1]["quiz"])
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function quiz_select(opt) {
    if (opt == quiz_data_list[quiz_data_list.length - 1]["quiz"]["answer"]) {
        if (!quiz_prompted) {
            $.ajax({
                type: 'GET',
                url: "api/quiz_passed",
                data:
                    {
                        word_index: quiz_word_index,
                    },
                error: function (XMLHttpRequest, textStatus, errorThrown) {
                    console.log(XMLHttpRequest.status);
                    console.log(XMLHttpRequest.readyState);
                    console.log(textStatus);
                }
            });
        }
        next_quiz()
        apply_quiz(quiz_data_list[quiz_data_list.length - 1]["quiz"])
    } else {
        if (!quiz_prompted) {
            $.ajax({
                type: 'GET',
                url: "api/quiz_failed",
                data:
                    {
                        word_index: quiz_word_index,
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

function get_explanation_panel(title, summary, body, actions, open) {
    actions = typeof actions !== 'undefined' ? actions : "";
    open = typeof open !== 'undefined' ? open : true;
    let ret = '';
    if(open)
        ret += '<div class="mdui-panel-item mdui-panel-item-open">';
    else
        ret += '<div class="mdui-panel-item">';
    ret += '<div class="mdui-panel-item-header">' +
        '<div class="mdui-panel-item-title">' + title + '</div>' +
        '<div class="mdui-panel-item-summary">' + summary + '</div>';
    if(!open)
       ret += '<i class="mdui-panel-item-arrow mdui-icon material-icons">keyboard_arrow_down</i>';
    ret += '</div><div class="mdui-panel-item-body">' + body;
    if (actions != "") {
        ret += '<div class="mdui-float-right">' + actions + '</div>';
    }
    ret += '</div></div>';
    return ret;
}

function prompt_explanation_panel(result, opt, open)
{
    let actions = "";
    if (result[opt]["is_marked"]) {
        actions = '<button class=\"mdui-btn mdui-ripple\" onclick=\"save_quiz_prompt_panel_status();unmark_word('
            + quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"][opt] + ');' +
            'quiz_prompt_data[\'' + opt + '\'][\'is_marked\']=false;init_prompt()\"><i class="mdui-icon material-icons">delete</i>取消收藏</button>';
    } else {
        actions = '<button class=\"mdui-btn mdui-ripple\" onclick=\"save_quiz_prompt_panel_status();mark_word('
            + quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"][opt] + ');' +
            'quiz_prompt_data[\'' + opt + '\'][\'is_marked\']=true;init_prompt()\"><i class="mdui-icon material-icons">star</i>收藏</button>';
    }
    return get_explanation_panel(opt, quiz_data_list[quiz_data_list.length - 1]["quiz"]["options"][opt],
        result[opt]["explanation"], actions, open);
}

function search_explanation_panel(words, pos) {
    let actions = "";
    if (words[pos]["is_marked"]) {
        actions = '<button class=\"mdui-btn mdui-ripple\" onclick=\"unmark_word('
            + words[pos]["word_index"] + ');' +
            'search_data[\'words\'][' + pos + '][\'is_marked\']=false;' +
            'init_search_result()\"><i class="mdui-icon material-icons">delete</i>取消收藏</button>';
    } else {
        actions = '<button class=\"mdui-btn mdui-ripple\" onclick=\"mark_word('
            + words[pos]["word_index"] + ');' +
            'search_data[\'words\'][' + pos + '][\'is_marked\']=true;' +
            'init_search_result()\"><i class="mdui-icon material-icons">star</i>收藏</button>';

    }
    return get_explanation_panel(words[pos]["word"],
        words[pos]["meaning"],
        words[pos]["explanation"],
        actions);
}


function marked_explanation_panel(word) {
    return get_explanation_panel(word["word"],
        word["meaning"],
        word["explanation"],
        '<button class=\"mdui-btn mdui-ripple\" onclick=\"unmark_word('
        + word["word_index"] + ');$(this).parent().parent().parent().fadeTo(\'normal\', 0.01, function(){$(this).slideUp(\'normal\', function() {$(this).remove();});});;\">' +
        '<i class="mdui-icon material-icons">delete</i>取消收藏</button>', false);
}

function passed_explanation_panel(word) {
    var actions = '<button class=\"mdui-btn mdui-ripple\" onclick=\"renew('
        + word["word_index"] + ');$(this).parent().parent().parent().' +
        'fadeTo(\'normal\', 0.01, function(){$(this).slideUp(\'normal\', function() {$(this).remove();});});' +
        'var cnt = parseInt($(\'#passed_word_count\')[0].innerText) - 1;\n' +
        'var width = (cnt / parseInt($(\'#word_count\')[0].innerText)) * 100 + \'%\';' +
        '$(\'#progress_bar\').attr(\'style\', \'width:\' + width);' +
        '$(\'#passed_word_count\').html(cnt);\">' +
        '<i class="mdui-icon material-icons">replay</i>重新背</button>';
    return get_explanation_panel(word["word"],
        word["meaning"],
        word["explanation"],
        actions, false);
}


function quiz_prompt(opt) {
    if (!quiz_prompted) {
        quiz_prompted = true;
        $("#" + opt).addClass("mdui-color-red");
        $.ajax({
            type: 'GET',
            url: "api/quiz_prompt",
            data:
                {
                    A_index: quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"]["A"],
                    B_index: quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"]["B"],
                    C_index: quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"]["C"],
                    D_index: quiz_data_list[quiz_data_list.length - 1]["quiz"]["indexes"]["D"],
                    word_index: quiz_word_index,
                },
            success: function (result) {
                quiz_prompt_data = result;
                if (result["status"] == "success") {
                    init_prompt()
                }
                else{
                    mdui.snackbar(result["message"])
                }
            },
            error: function (XMLHttpRequest, textStatus, errorThrown) {
                console.log(XMLHttpRequest.status);
                console.log(XMLHttpRequest.readyState);
                console.log(textStatus);
            }
        });
        $("#" + quiz_data_list[quiz_data_list.length - 1]["quiz"]["answer"]).addClass("mdui-color-green");
    }
}

function pass(word_index) {
    $.ajax({
        type: 'GET',
        url: "api/pass",
        data:
            {
                word_index: word_index,
            },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function renew(word_index) {
    $.ajax({
        type: 'GET',
        url: "api/renew",
        data:
            {
                word_index: word_index,
            },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function speak(word)
{
    var url = "https://api.oick.cn/txt/apiz.php?spd=5&text=" + encodeURI(word.replaceAll(' ', '-'));
    var n = new Audio(url);
    n.play();
}

function clear_records()
{
    $.ajax({
        type: 'GET',
        url: "api/clear_records",
        success: function (result) {
            quiz_prompt_data = result;
            if (result["status"] == "success") {
                init_passed();
                mdui.snackbar("已清除所有记录");
            }
            else{
                mdui.snackbar(result["message"]);
            }
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}

function clear_marks()
{
    $.ajax({
        type: 'GET',
        url: "api/clear_marks",
        success: function (result) {
            quiz_prompt_data = result;
            if (result["status"] == "success") {
                init_marked();
                mdui.snackbar("已清除所有收藏");
            }
            else{
                mdui.snackbar(result["message"]);
            }
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            console.log(XMLHttpRequest.status);
            console.log(XMLHttpRequest.readyState);
            console.log(textStatus);
        }
    });
}