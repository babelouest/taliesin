import React, { Component } from 'react';
import { Button, Modal, Row, Col, Label, FormControl, ToggleButton, ToggleButtonGroup } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class ModalEditStream extends Component {
  constructor(props) {
    super(props);
		
		this.state = {
			show: props.show, 
			dataSource: props.dataSource, 
			path: props.path,
			element: props.element,
			category: props.category, 
			categoryValue: props.categoryValue, 
			subCategory: props.subCategory, 
			subCategoryValue: props.subCategoryValue, 
			playlist: props.playlist,
			onCloseCb: props.onCloseCb,
			recursive: true,
			random: true,
			type: "jukebox",
			format: StateStore.getState().serverConfig.defrault_stream_format,
			channels: StateStore.getState().serverConfig.defrault_stream_channels,
			bitrate: StateStore.getState().serverConfig.defrault_stream_bitrate,
			sampleRate: StateStore.getState().serverConfig.defrault_stream_sample_rate,
			formatDisabled: false,
			channelsDisabled: false,
			bitrateDisabled: false,
			sampleRateDisabled: false,
			playNow: true
		};
		
		this.close = this.close.bind(this);
		this.handleChangeRecursive = this.handleChangeRecursive.bind(this);
		this.handleChangeRandom = this.handleChangeRandom.bind(this);
		this.handleChangeType = this.handleChangeType.bind(this);
		this.handleChangeFormat = this.handleChangeFormat.bind(this);
		this.handleChangeChannels = this.handleChangeChannels.bind(this);
		this.handleChangeBitrate = this.handleChangeBitrate.bind(this);
		this.handleChangeSampleRate = this.handleChangeSampleRate.bind(this);
		this.handleChangePlayNow = this.handleChangePlayNow.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			show: nextProps.show, 
			dataSource: nextProps.dataSource, 
			path: nextProps.path,
			element: nextProps.element,
			category: nextProps.category, 
			categoryValue: nextProps.categoryValue, 
			subCategory: nextProps.subCategory, 
			subCategoryValue: nextProps.subCategoryValue, 
			playlist: nextProps.playlist,
			onCloseCb: nextProps.onCloseCb,
			recursive: true,
			random: true,
			type: "jukebox",
			format: StateStore.getState().serverConfig.defrault_stream_format,
			channels: StateStore.getState().serverConfig.defrault_stream_channels,
			bitrate: StateStore.getState().serverConfig.defrault_stream_bitrate,
			sampleRate: StateStore.getState().serverConfig.defrault_stream_sample_rate,
			formatDisabled: false,
			channelsDisabled: false,
			bitrateDisabled: false,
			sampleRateDisabled: false,
			playNow: true
		});
	}

  close(result, e) {
		if (e) {
			e.preventDefault();
		}
		if (result) {
			this.state.onCloseCb({
				dataSource: this.state.dataSource, 
				element: this.state.element,
				path: this.state.path, 
				recursive: this.state.recursive,
				random: this.state.random,
				type: this.state.type,
				format: this.state.format,
				channels: this.state.channels,
				bitrate: this.state.bitrate,
				sampleRate: this.state.sampleRate
			});
		} else {
			this.state.onCloseCb(false);
		}
  }
	
	handleChangeRecursive() {
		this.setState({recursive: !this.state.recursive});
	}
	
	handleChangeRandom() {
		this.setState({random: !this.state.random});
	}
	
	handleChangeType(e) {
		var newStatus = {type: e.target.value};
		if (e.target.value === "webradio") {
			newStatus.format = "mp3";
			newStatus.formatDisabled = true;
		} else {
			newStatus.formatDisabled = false;
		}
		this.setState(newStatus);
	}
	
	handleChangeFormat(e) {
		var newStatus = {format: e.target.value};
		if (e.target.value === "flac") {
			newStatus.bitrateDisabled = true;
		} else {
			newStatus.bitrateDisabled = false;
		}
		this.setState(newStatus);
	}
	
	handleChangeChannels(e) {
		this.setState({channels: e.target.value});
	}
	
	handleChangeBitrate(e) {
		this.setState({bitrate: e.target.value});
	}
	
	handleChangeSampleRate(e) {
		this.setState({sampleRate: e.target.value});
	}
	
	handleChangePlayNow() {
		this.setState({playNow: !this.state.playNow});
	}
	
  render() {
		var recursive, random, path, playlist;
		if (this.state.element && this.state.element.type === "folder") {
			recursive = 
				<div>
					<Row>
						<Col md={4}>
							<Label>Recursive</Label>
						</Col>
						<Col md={8}>
							<ToggleButtonGroup type="checkbox">
								<ToggleButton
									value={this.state.recursive}
									onChange={this.handleChangeRecursive}>
										{this.state.recursive?"Recursive":"Non recursive"}
								</ToggleButton>
							</ToggleButtonGroup>
						</Col>
					</Row>
					<Row>
						<Col md={12}>
							<hr/>
						</Col>
					</Row>
				</div>;
		}
		if (this.state.path) {
			path =
				<div>
					<Row>
						<Col md={4}>
							<Label>Path</Label>
						</Col>
						<Col md={8}>
							{this.state.dataSource + " - " + this.state.path}
						</Col>
					</Row>
					<Row>
						<Col md={12}>
							<hr/>
						</Col>
					</Row>
				</div>;
		} else {
			var categoryPath = this.state.category + " / " + this.state.categoryValue;
			if (this.state.subCategory) {
				categoryPath += " / " + this.state.subCategory + " / " + this.state.subCategoryValue
			}
			path =
				<div>
					<Row>
						<Col md={4}>
							<Label>Category</Label>
						</Col>
						<Col md={8}>
							{categoryPath}
						</Col>
					</Row>
					<Row>
						<Col md={12}>
							<hr/>
						</Col>
					</Row>
				</div>;
		}
		if (this.state.playlist) {
			playlist =
				<div>
					<Row>
						<Col md={4}>
							<Label>Playlist</Label>
						</Col>
						<Col md={8}>
							{this.state.playlist.name}
						</Col>
					</Row>
					<Row>
						<Col md={12}>
							<hr/>
						</Col>
					</Row>
				</div>;
		}
		if (this.state.type === "webradio") {
			random = 
			<div>
				<Row>
					<Col md={4}>
						<Label>Random</Label>
					</Col>
					<Col md={8}>
						<ToggleButtonGroup type="checkbox">
							<ToggleButton
								value={this.state.random}
								onChange={this.handleChangeRandom}>
									{this.state.random?"Random":"Non random"}
							</ToggleButton>
						</ToggleButtonGroup>
					</Col>
				</Row>
				<Row>
					<Col md={12}>
						<hr/>
					</Col>
				</Row>
			</div>
		}
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>Create Stream</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					<form onSubmit={(e) => {this.close(true, e)}}>
						{playlist}
						{path}
						{recursive}
						<Row>
							<Col md={4}>
								<Label>Play as</Label>
							</Col>
							<Col md={8}>
								<FormControl componentClass="select" placeholder="select" value={this.state.type} onChange={this.handleChangeType}>
									<option value="jukebox">Jukebox</option>
									<option value="webradio">Webradio</option>
								</FormControl>
							</Col>
						</Row>
						<Row>
							<Col md={12}>
								<hr/>
							</Col>
						</Row>
						{random}
						<Row>
							<Col md={4}>
								<Label>Format</Label>
							</Col>
							<Col md={8}>
								<FormControl componentClass="select" placeholder="select" value={this.state.format} onChange={this.handleChangeFormat} disabled={this.state.formatDisabled}>
									<option value="mp3">MP3</option>
									<option value="vorbis">OGG/Vorbis</option>
									<option value="flac">FLAC</option>
								</FormControl>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>Channels</Label>
							</Col>
							<Col md={8}>
								<FormControl componentClass="select" placeholder="select" value={this.state.channels} onChange={this.handleChangeChannels} disabled={this.state.channelsDisabled}>
									<option value="1">Mono</option>
									<option value="2">Stereo</option>
								</FormControl>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>Bit rate</Label>
							</Col>
							<Col md={8}>
								<FormControl componentClass="select" placeholder="select" value={this.state.bitrate} onChange={this.handleChangeBitrate} disabled={this.state.bitrateDisabled}>
									<option value="32000">32 kbps</option>
									<option value="96000">96 kbps</option>
									<option value="128000">128 kbps</option>
									<option value="192000">192 kbps</option>
									<option value="256000">256 kbps</option>
									<option value="320000">320 kbps</option>
								</FormControl>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>Sample rate</Label>
							</Col>
							<Col md={8}>
								<FormControl componentClass="select" placeholder="select" value={this.state.sampleRate} onChange={this.handleChangeSampleRate} disabled={this.state.sampleRateDisabled}>
									<option value="8000">8 kHz</option>
									<option value="11025">11 kHz</option>
									<option value="22050">22 kHz</option>
									<option value="32000">32 kHz</option>
									<option value="44100">44.1 kHz</option>
									<option value="48000">48 kHz</option>
								</FormControl>
							</Col>
						</Row>
						<Row>
							<Col md={12}>
								<hr/>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>Play Now in selected player</Label>
							</Col>
							<Col md={8}>
								<ToggleButtonGroup type="checkbox">
									<ToggleButton
										value={this.state.playNow}
										onChange={this.handleChangePlayNow}>
											{this.state.playNow?"Yes":"No"}
									</ToggleButton>
								</ToggleButtonGroup>
							</Col>
						</Row>
					</form>
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)}>OK</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>Cancel</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalEditStream;
