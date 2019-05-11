/*
 v2m_adapter.js: Adapts "V2 by farbrausch" backend to generic WebAudio/ScriptProcessor player.
 
 version 1.0
 
 	Copyright (C) 2018 Juergen Wothke

 LICENSE
 
	The Artistic License 2.0 Copyright (c) 2000-2006, The Perl Foundation.
*/
V2MBackendAdapter = (function(){ var $this = function () {
		$this.base.call(this, backend_V2M.Module, 2);
		this._manualSetupComplete= true;
		this._undefined;
		this._currentPath;
		this._currentFile;
		this._scopeEnabled= false;
		
		if (!backend_V2M.Module.notReady) {
			// in sync scenario the "onRuntimeInitialized" has already fired before execution gets here,
			// i.e. it has to be called explicitly here (in async scenario "onRuntimeInitialized" will trigger
			// the call directly)
			this.doOnAdapterReady();
		}				
	}; 
	// sample buffer contains 2-byte integer sample data (i.e. 
	// must be rescaled) of 2 interleaved channels
	extend(EmsHEAP16BackendAdapter, $this, {
		enableScope: function(enable) {
			this._scopeEnabled= enable;
		},
		doOnAdapterReady: function() {
			// called when runtime is ready (e.g. asynchronously when WASM is loaded)
			// if FS needed to be setup of would be done here..
		},
		getAudioBuffer: function() {
			var ptr=  this.Module.ccall('emu_get_audio_buffer', 'number');			
			// make it a this.Module.HEAP16 pointer
			return ptr >> 1;	// 2 x 16 bit samples			
		},
		getAudioBufferLength: function() {
			var len= this.Module.ccall('emu_get_audio_buffer_length', 'number');
			return len;
		},
		computeAudioSamples: function() {
			return this.Module.ccall('emu_compute_audio_samples', 'number');
		},
		getMaxPlaybackPosition: function() { 
			return this.Module.ccall('emu_get_max_position', 'number');
		},
		getPlaybackPosition: function() {
			return this.Module.ccall('emu_get_current_position', 'number');
		},
		seekPlaybackPosition: function(pos) {
			this.Module.ccall('emu_seek_position', 'number', ['number'], [pos]);
		},		
		getPathAndFilename: function(filename) {
			var sp = filename.split('/');
			var fn = sp[sp.length-1];					
			var path= filename.substring(0, filename.lastIndexOf("/"));	
			if (path.lenght) path= path+"/";
			
			return [path, fn];
		},
		mapBackendFilename: function (name) {
			// "name" comes from the C++ side 
			var input= this.Module.Pointer_stringify(name);
			return input;
		},
		registerFileData: function(pathFilenameArray, data) {
			return this.registerEmscriptenFileData(pathFilenameArray, data);
		},
		loadMusicData: function(sampleRate, path, filename, data, options) {
			var buf = this.Module._malloc(data.length);
			this.Module.HEAPU8.set(data, buf);
			var ret = this.Module.ccall('emu_load_file', 'number', ['string', 'number', 'number'], [filename, buf, data.length]);
			this.Module._free(buf);

			if (ret == 0) {
				this.playerSampleRate = this.Module.ccall('emu_get_sample_rate', 'number');
				this.resetSampleRate(sampleRate, this.playerSampleRate);
				this._currentPath= path;
				this._currentFile= filename;
			} else {
				this._currentPath= this._undefined;
				this._currentFile= this._undefined;
			}
			return ret;			
		},
		evalTrackOptions: function(options) {
			if (typeof options.timeout != 'undefined') {
				ScriptNodePlayer.getInstance().setPlaybackTimeout(options.timeout*1000);
			} else {
				ScriptNodePlayer.getInstance().setPlaybackTimeout(-1);	// reset last songs setting
			}
			var id= (options && options.track) ? options.track : -1;	// by default do not set track
			
			return this.Module.ccall('emu_set_subsong', 'number', ['number', 'number'], [id, this._scopeEnabled]);
		},				
		teardown: function() {
			this.Module.ccall('emu_teardown', 'number');	// just in case
		},
		getSongInfoMeta: function() {
			return {title: String
					};
		},
		getNumberTraceStreams: function() {
			return this.Module.ccall('emu_number_trace_streams', 'number');			
		},
		getTraceStreams: function() {
			var result= [];
			var n= this.getNumberTraceStreams();

			var ret = this.Module.ccall('emu_get_trace_streams', 'number');			
			var array = this.Module.HEAP32.subarray(ret>>2, (ret>>2)+n);
			
			for (var i= 0; i<n; i++) {
				result.push(array[i] >> 2);	// pointer to float array
			}

			return result;
		},
		getTraceTitles: function() {
			var result= [];
			var n= this.getNumberTraceStreams();

			var ret = this.Module.ccall('emu_get_trace_titles', 'number');			
			var array = this.Module.HEAP32.subarray(ret>>2, (ret>>2)+n);
			
			for (var i= 0; i<n; i++) {
				result.push(array[i] >> 2);	// pointer to char array
			}

			return result;
		},
		// how the 64 voices are currently mapped to the available 16 channels
		getVoiceMapping: function() {
			var n= 64;
			var result=new Array(n);

			var ret = this.Module.ccall('emu_get_voice_map', 'number');			
			var array = this.Module.HEAP32.subarray(ret>>2, (ret>>2)+n);
			
			for (var i= 0; i<n; i++) {
				result.push(array[i]);	// int
			}

			return result;
		},

		readFloatTrace: function(buffer, idx) {
			// traces are already in the respective format
			return this.Module.HEAPF32[buffer+idx]/8;		// range of voice data is unclear.. but 8 seems about ok
		},
		
		updateSongInfo: function(filename, result) {
			var numAttr= 1;
			var ret = this.Module.ccall('emu_get_track_info', 'number');

			var array = this.Module.HEAP32.subarray(ret>>2, (ret>>2)+numAttr);
			result.title= this.Module.Pointer_stringify(array[0]);
		}
	});	return $this; })();