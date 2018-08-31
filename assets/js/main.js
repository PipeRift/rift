// jQuery for page scrolling feature - requires jQuery Easing plugin
$(function() {
    $('a.page-scroll').bind('click', function(event) {
        if (this.hash !== "")
        {
            var hash = this.hash;
            $('html, body').stop().animate({
                scrollTop: $(hash).offset().top
            }, 1500, 'easeInOutExpo');

            event.preventDefault();
        }
    });
});

$(document).ready(function(){
    $('[data-toggle="popover"]').popover();   
    $('[data-toggle="tooltip"]').tooltip();
});
