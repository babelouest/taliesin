import React, { Component } from 'react';
import { Image } from 'react-bootstrap';

import ModalMedia from '../Modal/ModalMedia';

class MediaInfo extends Component {
	constructor(props) {
		super(props);
		this.state = {
			media: props.media, 
			imgThumbBlob: props.imgThumbBlob, 
			title: this.buildTitle(props.media, props.index, props.total), 
			meta: props.meta,
			showModal: false
		};
		this.handleOpenModal = this.handleOpenModal.bind(this);
		this.handleCloseModal = this.handleCloseModal.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			media: nextProps.media, 
			title: this.buildTitle(nextProps.media, nextProps.index, nextProps.total), 
			imgThumbBlob: nextProps.imgThumbBlob
		});
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
			} else if (media.name) {
				title += media.name.replace(/\.[^/.]+$/, "");
			}
		}
		return title;
	}
	
	handleOpenModal() {
    if (this.state.media) {
      this.setState({showModal: true});
    }
	}
	
	handleCloseModal() {
		this.setState({showModal: false});
	}
	
	render() {
		var image = "";
		if (this.state.imgThumbBlob) {
			image = <Image src={"data:image/jpeg;base64," + this.state.imgThumbBlob} thumbnail responsive className="cover-image-thumb"/>;
		} else {
			image = <Image src="images/generic-album.png" thumbnail responsive className="cover-image-thumb"/>;
		}
		return (
			<div className="media-container">
				<div className="media-content hidden-sm hidden-xs">
					<a role="button" onClick={this.handleOpenModal} title={this.state.title}>
						<div>
							<label>{this.state.meta}</label>
						</div>
						<div className="scroll-left">
							<span>{this.state.title}</span>
						</div>
						<div className="text-center">
							{image}
						</div>
					</a>
				</div>
				<div className="visible-sm visible-xs text-center">
					<a role="button" onClick={this.handleOpenModal} title={this.state.title}>
						<div>
							<label>{this.state.meta}</label>
						</div>
						<div>
							<span>{this.state.title}</span>
						</div>
					</a>
				</div>
				<ModalMedia show={this.state.showModal} media={this.state.media} title={this.state.title} onClose={this.handleCloseModal} />
			</div>
		);
	}
}

export default MediaInfo;
