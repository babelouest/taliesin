import React, { Component } from 'react';
import StateStore from '../lib/StateStore';
import MediaInfo from './MediaInfo';

class JukeboxNow extends Component {
	constructor(props) {
		super(props);
		this.state = {
			folded: props.folded,
			imgThumbBlob: false, 
			media: props.media,
			index: props.index,
			total: props.total
		};
		this.loadCover();
		this.setPageTitle();
		
		this.loadCover = this.loadCover.bind(this);
		this.setPageTitle = this.setPageTitle.bind(this);
	}
	
	componentWillUnmount() {
		this.setState({media: false});
		this._ismounted = false;
	}
	
	componentDidMount() {
		this._ismounted = true;
	}
	
	componentWillReceiveProps(nextProps) {
		var newMedia = (this.state.media.data_source !== nextProps.media.data_source || this.state.media.path !== nextProps.media.path);
		this.setState({folded: nextProps.folded, media: (nextProps.media?nextProps.media:this.state.media), index: nextProps.index, total: nextProps.total}, () => {
			if (newMedia) {
				this.loadCover();
				this.setPageTitle();
			}
		});
	}
	
	loadCover() {
		if (this.state.media.data_source && this.state.media.path && !StateStore.getState().showFullScreen) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.media.data_source) + "/browse/path/" + encodeURI(this.state.media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
			.then((result) => {
				this.setState({imgThumbBlob: result});
			})
			.fail(() => {
				this.setState({imgThumbBlob: false});
			});
		}
	}
	
	setPageTitle() {
		if (this._ismounted) {
			if (this.state.media) {
				document.title = this.buildTitle(this.state.media) + " - Taliesin"
			} else {
				document.title = "Taliesin";
			}
		}
	}
	
	buildTitle(media, index, total) {
		var title = "";
		if (!!media) {
			if (index > -1) {
				title += ((index+1)<10?"0"+(index+1):(index+1)) + "/" + (total<10?"0"+total:total) + " - ";
			}
			if (!!media.tags) {
				if (index === -1) {
					if (media.tags.artist || media.tags.album_artist) {
						title += (media.tags.artist || media.tags.album_artist) + " - ";
					}
				}
				title += (media.tags.title || media.name.replace(/\.[^/.]+$/, ""));
			} else {
				title += media.name.replace(/\.[^/.]+$/, "");
			}
		}
		return title;
	}
	
	render() {
		return (
			<MediaInfo media={this.state.media} imgThumbBlob={this.state.imgThumbBlob} meta="Jukebox" folded={this.state.folded} index={this.state.index} total={this.state.total}/>
		);
	}
}

export default JukeboxNow;
