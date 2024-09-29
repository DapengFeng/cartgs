(function($) {

    $.fn.twentytwentyvid = function(options) {
        var options = $.extend({
            default_offset_pct_x: 0.5,
            default_offset_pct_y: 0.5,
            click_needed: false,
        }, options);
        return this.each(function() {

            var sliderPctX = options.default_offset_pct_x;
            var sliderPctY = options.default_offset_pct_y;
            var container = $(this);
            var clickNeeded = options.click_needed;


            var numImgs = container.children("div").length;
            var imgs = [container.children("div:eq(0)"),
                        container.children("div:eq(1)"),
                        container.children("div:eq(2)"),
                        container.children("div:eq(3)")];
            var videos = [imgs[0].children("video:eq(0)"),
                          imgs[1].children("video:eq(0)"),
                          imgs[2].children("video:eq(0)"),
                          imgs[3].children("video:eq(0)")];

            var temp = 0;
            var referenceIndex = 0;
            for (var i = 0; i < numImgs; i++)
            {
                if (temp < videos[i].width())
                {
                    temp = videos[i].width();
                    referenceIndex = i;
                }
            }

            container.css("max-width", videos[referenceIndex].get(0).videoWidth);
            console.log("max-width: " + videos[referenceIndex].get(0).videoWidth);
            container.addClass("twentytwenty-vid-compare-" + numImgs)

            for (var i = 0; i < numImgs; i++)
                imgs[i].addClass("twentytwenty-vid-" + (i+1));

            container.append("<div class='twentytwenty-vid-overlay'></div>");
            var overlay = container.find(".twentytwenty-vid-overlay");


            for (var i = 0; i < numImgs; i++)
                overlay.append("<div class='twentytwenty-vid-label-" + (i+1) + "'>" + imgs[i].attr('alt') + "</div>");

            var labels = [overlay.find(".twentytwenty-vid-label-1"),
                          overlay.find(".twentytwenty-vid-label-2"),
                          overlay.find(".twentytwenty-vid-label-3"),
                          overlay.find(".twentytwenty-vid-label-4")];

            for (var i = 0; i < numImgs; i++)
                overlay.append("<div class='twentytwenty-vid-frame-" + (i+1) + "'></div>");

            var frames = [overlay.find(".twentytwenty-vid-frame-1"),
                          overlay.find(".twentytwenty-vid-frame-2"),
                          overlay.find(".twentytwenty-vid-frame-3"),
                          overlay.find(".twentytwenty-vid-frame-4")];


            // container.append("<div class='twentytwenty-vid-handle'></div>");
            // var slider = container.find(".twentytwenty-vid-handle");
            // if (sliderOrientation === 'vertical') {
            //     slider.append("<span class='twentytwenty-vid-up-arrow'></span>");
            //     slider.append("<span class='twentytwenty-vid-down-arrow'></span>");
            // }
            // else if (sliderOrientation === 'horizontal') {
            //     slider.append("<span class='twentytwenty-vid-left-arrow'></span>");
            //     slider.append("<span class='twentytwenty-vid-right-arrow'></span>");
            // }
            // else if (sliderOrientation === 'cross') {
            //     slider.append("<span class='twentytwenty-vid-up-arrow'></span>");
            //     slider.append("<span class='twentytwenty-vid-down-arrow'></span>");
            //     slider.append("<span class='twentytwenty-vid-left-arrow'></span>");
            //     slider.append("<span class='twentytwenty-vid-right-arrow'></span>");
            // }

            var calcOffset = function(dimensionPctX, dimensionPctY) {
                var w = videos[referenceIndex].width();
                var h = videos[referenceIndex].height();
                return {
                    w: w + "px",
                    h: h + "px",
                    cw: (dimensionPctX * w) + "px",
                    ch: (dimensionPctY * h) + "px",
                    w2: w,
                    h2: h,
                    cw2: (dimensionPctX * w),
                    ch2: (dimensionPctY * h)
                };
            };

            var adjustContainer = function(offset) {
                overlay.css("width", offset.w);
                overlay.css("height", offset.h);
                if (numImgs == 2)
                {
                    imgs[0].css("clip", "rect(0," + offset.cw + "," + offset.h + ",0)");

                    labels[1].css({left: offset.cw2});
                    labels[0].css({right: offset.w2 - offset.cw2});


                    frames[0].css({width: offset.cw2, height: offset.h2});
                    frames[1].css({width: offset.w2 - offset.cw2, height: offset.h2});
                }
                else if (numImgs == 3)
                {
                    imgs[0].css("clip", "rect(0," + offset.cw + "," + offset.ch + ",0)");
                    imgs[1].css("clip", "rect(0," + offset.w + "," + offset.ch + "," + offset.cw + ")");
                    imgs[2].css("clip", "rect(" + offset.ch + "," + offset.w + "," + offset.h + ",0)");

                    frames[0].css({width: offset.cw2, height: offset.ch2});
                    frames[1].css({width: offset.w2 - offset.cw2, height: offset.ch2});
                    frames[2].css({width: offset.w2, height: offset.h2 - offset.ch2});

                    labels[0].css({right: offset.w2 - offset.cw2, bottom: offset.h2 - offset.ch2});
                    labels[1].css({left: offset.cw2, bottom: offset.h2 - offset.ch2});
                    labels[2].css({top: offset.ch2});
                }
                else if (numImgs == 4)
                {
                    imgs[0].css("clip", "rect(0," + offset.cw + "," + offset.ch + ",0)");
                    imgs[1].css("clip", "rect(0," + offset.w + "," + offset.ch + "," + offset.cw + ")");
                    imgs[2].css("clip", "rect(" + offset.ch + "," + offset.cw + "," + offset.h + ",0)");
                    imgs[3].css("clip", "rect(" + offset.ch + "," + offset.w + "," + offset.h + "," + offset.cw + ")");

                    frames[0].css({width: offset.cw2, height: offset.ch2});
                    frames[1].css({width: offset.w2 - offset.cw2, height: offset.ch2});
                    frames[2].css({width: offset.cw2, height: offset.h2 - offset.ch2});
                    frames[3].css({width: offset.w2 - offset.cw2, height: offset.h2 - offset.ch2});

                    labels[0].css({right: offset.w2 - offset.cw2, bottom: offset.h2 - offset.ch2});
                    labels[1].css({left: offset.cw2, bottom: offset.h2 - offset.ch2});
                    labels[2].css({right: offset.w2 - offset.cw2, top: offset.ch2});
                    labels[3].css({left: offset.cw2, top: offset.ch2});

                }

                container.css("height", offset.h);
            };

            function video_click() {
                if (videos[referenceIndex].get(0).paused){
                    console.log("play");
                    for(var i = 0; i < numImgs; i++)
                        videos[i].get(0).play();
                }
                else{
                    console.log("pause");
                    for(var i = 0; i < numImgs; i++)
                        videos[i].get(0).pause();
                }
                // v.paused ? v.play() : v.pause();
                // v2.paused ? v2.play() : v2.pause();
                for (var i = 0; i < numImgs; i++)
                    videos[i].get(0).currentTime=videos[referenceIndex].get(0).currentTime;
            }

            var adjustSlider = function(pctX, pctY) {
                var offset = calcOffset(pctX, pctY);

                // if (sliderOrientation === 'vertical')
                //     slider.css("top", offset.ch);
                // else if (sliderOrientation === 'horizontal')
                //     slider.css("left", offset.cw);
                // else if (sliderOrientation === 'cross')
                // {
                //     slider.css("top", offset.ch);
                //     slider.css("left", offset.cw);
                // }

                adjustContainer(offset);
            }

            $(window).on("resize.twentytwenty", function(e) {

                for (var i = 0; i < numImgs; i++)
                {
                    if (i != referenceIndex)
                    {
                        imgs[i].css("width", imgs[referenceIndex].width());
                        imgs[i].css("height", imgs[referenceIndex].height());
                    }
                }
                adjustSlider(sliderPctX, sliderPctY);
            });


            // var offsetX = 0;
            // var offsetY = 0;
            // var imgWidth = 0;
            // var imgHeight = 0;

            // container.on("movestart", function(e) {
            //     container.addClass("active");
            //     offsetX = container.offset().left;
            //     offsetY = container.offset().top;
            //     imgWidth = imgs[0].width();
            //     imgHeight = imgs[0].height();
            // });

            // container.on("moveend", function(e) {
            //     container.removeClass("active");
            // });

            // container.on("move", function(e) {
            //     if (container.hasClass("active")) {
            //         sliderPctX = Math.max(0, Math.min(1, (e.pageX - offsetX) / imgWidth));
            //         sliderPctY = Math.max(0, Math.min(1, (e.pageY - offsetY) / imgHeight));

            //         adjustSlider(sliderPctX, sliderPctY);
            //     }
            // });

            container.on("move mousemove", function(e) {
                    sliderPctX = Math.max(0, Math.min(1, (e.pageX - container.offset().left) / imgs[referenceIndex].width()));
                    sliderPctY = Math.max(0, Math.min(1, (e.pageY - container.offset().top) / imgs[referenceIndex].height()));
                    adjustSlider(sliderPctX, sliderPctY);
            });

            container.on("mousedown", function(event) {
                video_click();
                event.preventDefault();
            });

            $(window).onload = function(){
                for (var i = 0; i < numImgs; i++) {
                    videos[referenceIndex].get(0).addEventListener("seeking", function() {
                        videos[i].get(0).currentTime = videos[referenceIndex].get(0).currentTime;
                    });
                    videos[referenceIndex].get(0).addEventListener("seeked", function() {
                        videos[i].get(0).currentTime = videos[referenceIndex].get(0).currentTime;
                    });
                }
            }

            $(window).trigger("resize.twentytwenty");
        });
    };

})(jQuery);
