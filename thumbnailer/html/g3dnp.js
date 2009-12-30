var g3d = {};

g3d.NP = function (basename, suffix,
	minx, maxx, stepx, basex,
	miny, maxy, stepy, basey) {

	this.basename = basename;
	this.suffix = suffix;
	this.minx = minx;
	this.maxx = maxx;
	this.stepx = Math.max(stepx, 1);
	this.basex = basex;
	this.miny = miny;
	this.maxy = maxy;
	this.stepy = Math.max(stepy, 1);
	this.basey = basey;
	this.numImages = 0;

	var ix = 0;
	var iy = 0;

	this.steps = new Array();

	for(var y = miny; y <= maxy; y += this.stepy) {
		ix = 0;
		this.steps[iy] = new Array();

		for(var x = minx; x <= maxx; x += this.stepx) {
			this.steps[iy][ix] = new Image();
			ix ++;
			this.numImages ++;
		}

		iy ++;
	}

	this.maxx_i = ix;
	this.maxy_i = iy;

	this.curx_i = 0;
	this.cury_i = 0;

	this.animEnabled = false;
	this.animTimeout = 200;
};

g3d.NP.prototype.load = function (progressDivId) {
	var ix = 0;
	var iy = 0;

	this.progress = 0;

	var updateProgress = function (pDivId, self, idxx, idxy) {
		if(self.steps[idxy][idxx].complete)
			self.progress ++;
		var pDiv = document.getElementById(pDivId);
		if(pDiv) {
			var progress = (self.progress / self.numImages) | 0;
			pdiv.style.width = progress.toString() + '%';
		}
	};

	for(var y = this.miny; y <= this.maxy; y += this.stepy) {
		ix = 0;
		for(var x = this.minx; x <= this.maxx; x += this.stepx) {
			this.steps[iy][ix].src = this.basename +
				'_' + x + '_' + y + '.' + this.suffix;
			setTimeout(updateProgress, 2,
				progressDivId, this, ix, iy);
			ix ++;
		}
		iy ++;
	}
};

g3d.NP.prototype.insert = function (divId) {
	this.divId = divId;
	div = document.getElementById(divId);
	if (div) {
		var src = this.steps[this.cury_i][this.curx_i].src;

		var img = document.createElement("img");
		img.setAttribute("id", divId + '0');
		div.appendChild(img);

		var pDivId = divId + 'Progress';
		var pDiv = document.createElement("div");
		pDiv.setAttribute("style",
			"width: 100%; height: 10px; background-color: red;");
		div.appendChild(pDiv);

		this.load(pDivId);
		img.setAttribute("src", src);
	}

};

g3d.NP.prototype.animationSetTimeout = function (timeout) {
	this.animTimeout = timeout;
};

g3d.NP.prototype.animationStart = function () {
	self = this;
	if(typeof self['divId'] == 'undefined')
		self = this.g3dnp;

	if(!self.animEnabled) {
		self.animEnabled = true;
	setTimeout(self._next_frame, self.animTimeout, self.divId + '0',
		self);
	}
};

g3d.NP.prototype.animationStop = function () {
	self = this;
	if(typeof self['divId'] == 'undefined')
		self = this.g3dnp;

	self.animEnabled = false;
};

g3d.NP.prototype._next_frame = function (imgId, self) {
	if(!self.animEnabled)
		return;
	var img = document.getElementById(imgId);
	if(img) {
		self.cury_i ++;
		if(self.cury_i >= self.maxy_i) {
			self.cury_i = 0;
			self.curx_i ++;
			if(self.curx_i >= self.maxx_i) {
				self.curx_i = 0;
			}
		}
		img.src = self.steps[self.cury_i][self.curx_i].src;
		setTimeout(self._next_frame, self.animTimeout, imgId, self);
	}
};

g3d.NP.prototype.mouseDownHandler = function (event) {
	self = this.g3dnp;
};

g3d.NP.prototype.mouseUpHandler = function (event) {
	self = this.g3dnp;
	// this: div
	// self: G3DNP object
	//alert(event.keycode);
};

g3d.NP.prototype.addTools = function () {
	var div = document.getElementById(this.divId);
	var btn;

	btn = document.createElement("a");
	btn.g3dnp = this;
	btn.className = "tool";
	btn.href = "#";
	btn.innerHTML = "&#x220e;";
	btn.addEventListener("click", this.animationStop, false);
	div.appendChild(btn);

	btn = document.createElement("a");
	btn.g3dnp = this;
	btn.className = "tool";
	btn.href = "#";
	btn.innerHTML = "&#x2023";
	btn.addEventListener("click", this.animationStart, false);
	div.appendChild(btn);

};

g3d.NP.prototype.registerEventHandlers = function (divId) {
	if(typeof divId != 'undefined')
		div = document.getElementById(divId);
	else
		div = document.getElementById(this.divId);

	if(div) {
		div.g3dnp = this;
		div.addEventListener("mousedown", this.mouseDownHandler,
			false);
		div.addEventListener("mouseup", this.mouseUpHandler,
			false);				
	}
};

