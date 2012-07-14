$(document).ready(function() {
        $('form').submit(function(e) {
		e.preventDefault();

		/* populate quad objects (x pos, y pos, width, height and rotation) */
		var quad1 = {'x': parseInt($('#quad1 #x').val()), 'y': parseInt($('#quad1 #y').val()), 'w': parseInt($('#quad1 #w').val()), 'h': parseInt($('#quad1 #h').val()), 'r': parseInt($('#quad1 #r').val())};
		var quad2 = {'x': parseInt($('#quad2 #x').val()), 'y': parseInt($('#quad2 #y').val()), 'w': parseInt($('#quad2 #w').val()), 'h': parseInt($('#quad2 #h').val()), 'r': parseInt($('#quad2 #r').val())};

		/* display these in an svg element on the page */
		var SVG = '<svg width="400px"  height="400px">';
		/* draw quad 1 */
		SVG += '<rect x="' + (quad1.x - (quad1.w/2)) + '" y="' + (quad1.y - (quad1.h/2)) + '" width="' + quad1.w + '" height="' + quad1.h + '" ';
		SVG += ' transform = "rotate(' + quad1.r + ' ' + quad1.x + ' ' + quad1.y + ')"';
		SVG += ' style="fill:none; stroke: blue; stroke-width: 1;" />';
		/* draw quad 2 */
		SVG += '<rect x="' + (quad2.x - (quad2.w/2)) + '" y="' + (quad2.y - (quad2.h/2)) + '" width="' + quad2.w + '" height="' + quad2.h + '" ';
		SVG += ' transform = "rotate(' + quad2.r + ' ' + quad2.x + ' ' + quad2.y + ')"';
		SVG += ' style="fill:none; stroke: red; stroke-width: 1;" />';
		SVG += '</svg>';
		$('#svg').html(SVG);

		/* calculate the bouding box of these quads
		   NOTE: this is only really required if using rotation steps not equal to π/2
		   otherwise you can just use centerx [+-] width and centery [+-] height
		 */
		/* bounding box for quad1 & quad2 */
		var bound1 = fitRect(quad1.x, quad1.y, quad1.w, quad1.h, quad1.r);
		var bound2 = fitRect(quad2.x, quad2.y, quad2.w, quad2.h, quad2.r);

		/* check planes separately then if collision in all planes a collision has occured
		 */
		/* check collisions */
		var x = (bound1.x_max > bound2.x_min) && (bound1.x_min < bound2.x_max);
		var y = (bound1.y_max > bound2.y_min) && (bound1.y_min < bound2.y_max);
		if(x && y) {
		    $('#outcome').html('collision');
		}
		else {
		    $('#outcome').html('no collision');
		}
		return true;
	    });
    });

/* return bounding box of rectangle */
var fitRect = function( cx,cy,rw,rh,degrees ){
    var radians = degrees * (Math.PI/180);
    var x1 = -rw/2,
    x2 = rw/2,
    x3 = rw/2,
    x4 = -rw/2,
    y1 = rh/2,
    y2 = rh/2,
    y3 = -rh/2,
    y4 = -rh/2;

    var x11 = x1 * Math.cos(radians) + y1 * Math.sin(radians),
    y11 = -x1 * Math.sin(radians) + y1 * Math.cos(radians),
    x21 = x2 * Math.cos(radians) + y2 * Math.sin(radians),
    y21 = -x2 * Math.sin(radians) + y2 * Math.cos(radians),
    x31 = x3 * Math.cos(radians) + y3 * Math.sin(radians),
    y31 = -x3 * Math.sin(radians) + y3 * Math.cos(radians),
    x41 = x4 * Math.cos(radians) + y4 * Math.sin(radians),
    y41 = -x4 * Math.sin(radians) + y4 * Math.cos(radians);

    var x_min = Math.min(x11,x21,x31,x41),
    x_max = Math.max(x11,x21,x31,x41);

    var y_min = Math.min(y11,y21,y31,y41);
    y_max = Math.max(y11,y21,y31,y41);

    return {'x_max': cx+x_max, 'x_min': cx+x_min, 'y_max': cy+y_max, 'y_min': cy+y_min};
};

/* return distance between two points */
/* x1, y1, x2 and y2 are coordinates */
var distance = function(x1, y1, x2, y2) {
    return Math.sqrt( Math.pow((x2 - x1), 2) + Math.pow((y2 - y1), 2));
};

/* return angle between 2 lines */
var angleBetween2Lines = function(line1, line2) {
    var angle1 = Math.atan2(line1.y2-line1.y1, line1.x2-line1.x1);
    var angle2 = Math.atan2(line2.y2-line2.y1, line2.x2-line2.x1);
    return angle1 - angle2;
};
