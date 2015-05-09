$(document).ready(function(){
    loadcontent();
    $('.reportbug').click( function(){
	window.open('https://belle2.cc.kek.jp/redmine/projects/support/issues/new');
    });


    $('.revisions').click(function() {
            $('#revisions').slideToggle("fast");
    });
    $('.packages').click(function() {
            $('#packages').slideToggle("fast");
    });
    $('.filters').click(function() {
            $('#filters').slideToggle("fast");
    });
    $('.options').click(function() {
            $('#options').slideToggle("fast");
    });

    $('#packages').load('packages.html', function() {
	$('input[type="checkbox"][name="packages"]').click(function(){
            $("#"+$(this).attr("value")).toggle();
	});
    });

    $('input[type="checkbox"][name="options"]').click(function(){
        $(".descriptions").toggle();
    });
    $('input[type="checkbox"][name="overview"]').click(function(){
		if($(this).prop("checked")){
			$(".nomatrix").each(function(){$(this).removeClass("show");});
		} else {
			$(".nomatrix").each(function(){$(this).addClass("show");});
		}
    });
    $('input[type="checkbox"][name="showlists"]').click(function(){
		if(!$(this).prop("checked")){
			$(".hideme").each(function(){$(this).css("display", "none");});
			$(".hidetext").each(function(){$(this).text("(show)");});
		} else {
			$(".hideme").each(function(){$(this).css("display", "block");});
			$(".hidetext").each(function(){$(this).text("(hide)");});
		}
	});

});
