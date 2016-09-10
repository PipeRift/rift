---
layout: null
---
$(document).ready(function () {
  $('a.blog-button').click(function (e) {
    if ($('.panel-cover').hasClass('panel-cover--collapsed')) return
    currentWidth = $('.panel-cover').width()
    if (currentWidth < 960) {
      $('.panel-cover').addClass('panel-cover--collapsed')
      $('.content-wrapper').addClass('animated slideInRight')
    } else {
      console.log("opened");
      $('.panel-cover').css('max-width', currentWidth)
      $('.panel-cover').animate({'max-width': '530px', 'width': '40%'}, 400, swing = 'swing', function () {})
    }
  })

  if (window.location.hash && window.location.hash == '#blog') {
    $('.panel-cover').addClass('panel-cover--collapsed')
  }

  if (window.location.pathname !== '{{ site.baseurl }}' && window.location.pathname !== '{{ site.baseurl }}index.html') {
    $('.panel-cover').addClass('panel-cover--collapsed')
  }

  $('.btn-mobile-menu').click(function () {
    $('.navigation-wrapper').toggleClass('visible animated bounceInDown')
    $('.btn-mobile-menu__icon').toggleClass('icon-list icon-x-circle animated fadeIn')
  })

  $('.navigation-wrapper .blog-button').click(function () {
    $('.navigation-wrapper').toggleClass('visible')
    $('.btn-mobile-menu__icon').toggleClass('icon-list icon-x-circle animated fadeIn')
  })

  if (typeof(Storage) !== "undefined") {
    Lenguaje.saveDefaultOrSaved();

    $('.lenguaje-switch').click(function (e) {
      e.preventDefault();
      Lenguaje.setLenguaje(Lenguaje.isEnglish()? "spanish" : "english");
    })
  } else {
    console.log("No Web Storage support..");
    Lenguaje.setLenguaje("english");
  }

})

var Lenguaje = {

  saveDefaultOrSaved() {
    var lenguajeParam = getParameterByName("ln");
    if(lenguajeParam) {
      if(lenguajeParam == "en") {
        this.setLenguaje("english");
        return true;
      }
      if(lenguajeParam == "es")  {
        this.setLenguaje("spanish");
        return true;
      }
    }

    if(!localStorage.lenguaje) {
      this.setLenguaje("english");
      return true;
    }
    this.setLenguaje(localStorage.lenguaje);
   return false;
  },

  isEnglish() {
    return localStorage.lenguaje == "english"
  },

  setLenguaje(name) {
    if(typeof(Storage) !== "undefined")
      localStorage.lenguaje = name;

    $('.lenguaje').removeClass('visible');
    console.log('.lenguaje.'+name);
    $('.lenguaje.'+name).addClass('visible');
    this.updateName();
  },

  updateName () {
    if(typeof(Storage) !== "undefined") {
      $('.lenguaje-switch').html(this.isEnglish()? "Español" : "English");
      return;
    }
    $('.lenguaje-switch').html("Español");
  }
}

function getParameterByName(name, url) {
    if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, "\\$&");
    var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)"),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, " "));
}