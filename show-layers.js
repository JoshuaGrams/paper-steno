function showLayers(eltID, layers) {
	if(typeof layers === 'string') {
		var l = {}; l[layers] = true;
		layers = l;
	}
	var elt = document.getElementById(eltID);
	elt.addEventListener('load', function(){
		var svg = elt.getSVGDocument();
		var groups = svg.querySelectorAll('g[*|groupmode="layer"]');
		for(let i=0; i<groups.length; ++i) {
			var g = groups[i];
			var label = g.getAttribute('inkscape:label');
			var opacity = layers[label];
			if(opacity) {
				g.style.display = 'inline';
				if(opacity !== true) g.style.opacity = opacity;
			} else {
				g.style.display = 'none';
			}
		}
	});
}
