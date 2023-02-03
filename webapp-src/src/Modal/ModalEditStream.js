import React, { Component } from 'react';
import { Button, Modal, Row, Col, Label, FormControl, Checkbox } from 'react-bootstrap';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';
import config from '../lib/ConfigManager';

class ModalEditStream extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			name: "",
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
      streamUrl: "",
			recursive: true,
			random: true,
			icecast: false,
			type: "jukebox",
      scope: "me",
			format: StateStore.getState().serverConfig.default_stream_format,
			channels: StateStore.getState().serverConfig.default_stream_channels,
			bitrate: StateStore.getState().serverConfig.default_stream_bitrate,
			sampleRate: StateStore.getState().serverConfig.default_stream_sample_rate,
			formatDisabled: false,
			channelsDisabled: false,
			bitrateDisabled: false,
			sampleRateDisabled: false,
			playNow: true
		};
		
		this.close = this.close.bind(this);
		this.handleChangeName = this.handleChangeName.bind(this);
		this.handleChangeUrl = this.handleChangeUrl.bind(this);
		this.handleChangeRecursive = this.handleChangeRecursive.bind(this);
		this.handleChangeRandom = this.handleChangeRandom.bind(this);
		this.handleChangeIcecast = this.handleChangeIcecast.bind(this);
		this.handleChangeType = this.handleChangeType.bind(this);
		this.handleChangeFormat = this.handleChangeFormat.bind(this);
		this.handleChangeChannels = this.handleChangeChannels.bind(this);
		this.handleChangeBitrate = this.handleChangeBitrate.bind(this);
		this.handleChangeSampleRate = this.handleChangeSampleRate.bind(this);
		this.handleChangePlayNow = this.handleChangePlayNow.bind(this);
		this.handleChangeScope = this.handleChangeScope.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			name: "",
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
      streamUrl: "",
			recursive: true,
			random: true,
			icecast: false,
			type: "jukebox",
			format: StateStore.getState().serverConfig.default_stream_format,
			channels: StateStore.getState().serverConfig.default_stream_channels,
			bitrate: StateStore.getState().serverConfig.default_stream_bitrate,
			sampleRate: StateStore.getState().serverConfig.default_stream_sample_rate,
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
				name: this.state.name,
				dataSource: this.state.dataSource, 
				element: this.state.element,
				path: this.state.path, 
        streamUrl: this.state.streamUrl,
				recursive: this.state.recursive,
				random: this.state.random,
				type: this.state.type,
				format: this.state.format,
				channels: this.state.channels,
				bitrate: this.state.bitrate,
				sampleRate: this.state.sampleRate,
				playNow: this.state.playNow,
        scope: this.state.scope,
        icecast: this.state.icecast
			});
		} else {
			this.state.onCloseCb(false);
		}
	}
	
	handleChangeName(e) {
		this.setState({name: e.target.value});
	}
	
	handleChangeUrl(e) {
		this.setState({streamUrl: e.target.value});
	}
	
	handleChangeRecursive() {
		this.setState({recursive: !this.state.recursive});
	}
	
	handleChangeRandom() {
		this.setState({random: !this.state.random});
	}
	
	handleChangeIcecast() {
		this.setState({icecast: !this.state.icecast});
	}
	
	handleChangeType(e) {
		var newStatus = {type: e.target.value};
		if (e.target.value === "webradio") {
      if (this.state.format === "flac") {
        newStatus.format = "mp3";
        newStatus.bitrateDisabled = false;
      }
		}
		this.setState(newStatus);
	}
	
	handleChangeFormat(e) {
		var newStatus = {format: e.target.value};
			var serverConfig = config.getLocalConfigValue("serverConfig");
			if (serverConfig) {
				serverConfig.default_stream_format = e.target.value;
				config.setLocalConfigValue("serverConfig", serverConfig);
				StateStore.dispatch({type: "setServerConfig", config: serverConfig});
			}
		if (e.target.value === "flac") {
			newStatus.bitrateDisabled = true;
		} else {
			newStatus.bitrateDisabled = false;
		}
		this.setState(newStatus);
	}
	
	handleChangeScope(e) {
		this.setState({ scope: e.target.value });
	}
	
	handleChangeChannels(e) {
		var serverConfig = config.getLocalConfigValue("serverConfig");
		if (serverConfig) {
			serverConfig.default_stream_channels = e.target.value;
			config.setLocalConfigValue("serverConfig", serverConfig);
			StateStore.dispatch({type: "setServerConfig", config: serverConfig});
		}
		this.setState({channels: e.target.value});
	}
	
	handleChangeBitrate(e) {
		var serverConfig = config.getLocalConfigValue("serverConfig");
		if (serverConfig) {
			serverConfig.default_stream_bitrate = e.target.value;
			config.setLocalConfigValue("serverConfig", serverConfig);
			StateStore.dispatch({type: "setServerConfig", config: serverConfig});
		}
		this.setState({bitrate: e.target.value});
	}
	
	handleChangeSampleRate(e) {
		var serverConfig = config.getLocalConfigValue("serverConfig");
		if (serverConfig) {
			serverConfig.default_stream_sample_rate = e.target.value;
			config.setLocalConfigValue("serverConfig", serverConfig);
			StateStore.dispatch({type: "setServerConfig", config: serverConfig});
		}
		this.setState({sampleRate: e.target.value});
	}
	
	handleChangePlayNow() {
		this.setState({playNow: !this.state.playNow});
	}
	
	render() {
		var recursive, random, icecast, path,  formatSelect;
		if (this.state.element && this.state.element.type === "folder") {
			recursive = 
				<div>
					<Row>
						<Col md={4}>
							<Label>{i18n.t("common.recursive")}</Label>
						</Col>
						<Col md={8}>
              <Checkbox checked={this.state.recursive} onChange={this.handleChangeRecursive}>
              </Checkbox>
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
							<Label>{i18n.t("common.path")}</Label>
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
		} else if (this.state.playlist) {
			path =
				<div>
					<Row>
						<Col md={4}>
							<Label>{i18n.t("common.playlist")}</Label>
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
		} else {
			var categoryPath = this.state.category + " / " + this.state.categoryValue;
			if (this.state.subCategory) {
				categoryPath += " / " + this.state.subCategory + " / " + this.state.subCategoryValue
			}
			path =
				<div>
					<Row>
						<Col md={4}>
							<Label>{i18n.t("common.category")}</Label>
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
		if (this.state.type === "webradio") {
      formatSelect =
        <FormControl componentClass="select"
                     placeholder={i18n.t("common.select")}
                     value={this.state.format}
                     onChange={this.handleChangeFormat}
                     disabled={this.state.formatDisabled}>
          <option value="mp3">{i18n.t("common.format_mp3")}</option>
          <option value="vorbis">{i18n.t("common.format_ogg")}</option>
        </FormControl>
			random = 
			<div>
				<Row>
					<Col md={4}>
						<Label>{i18n.t("common.random")}</Label>
					</Col>
					<Col md={8}>
            <Checkbox checked={this.state.random} onChange={this.handleChangeRandom}>
            </Checkbox>
					</Col>
				</Row>
				<Row>
					<Col md={12}>
						<hr/>
					</Col>
				</Row>
			</div>
      if (StateStore.getState().serverConfig.icecast) {
        icecast =
        <div>
          <Row>
            <Col md={4}>
              <Label>{i18n.t("common.icecast")}</Label>
            </Col>
            <Col md={8}>
              <Checkbox checked={this.state.icecast} onChange={this.handleChangeIcecast} disabled={true}>
              </Checkbox>
            </Col>
          </Row>
          <Row>
            <Col md={12}>
              <hr/>
            </Col>
          </Row>
        </div>
      }
		} else {
      formatSelect =
        <FormControl componentClass="select"
                     placeholder={i18n.t("common.select")}
                     value={this.state.format}
                     onChange={this.handleChangeFormat}
                     disabled={this.state.formatDisabled}>
          <option value="mp3">{i18n.t("common.format_mp3")}</option>
          <option value="vorbis">{i18n.t("common.format_ogg")}</option>
          <option value="flac">{i18n.t("common.format_flac")}</option>
        </FormControl>
    }
		var scopeInput;
		if (StateStore.getState().profile.isAdmin) {
			scopeInput =
				<FormControl
					value={this.state.scope}
					onChange={this.handleChangeScope}
					componentClass="select"
					placeholder={i18n.t("common.select")}>
					<option value={"me"}>{i18n.t("common.scope_me")}</option>
					<option value={"all"}>{i18n.t("common.scope_all")}</option>
				</FormControl>;
		} else {
      scopeInput = <span>{this.state.scope==="me"?i18n.t("common.scope_me"):i18n.t("common.scope_all")}</span>;
		}
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>{i18n.t("modal.create_stream")}</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					<form onSubmit={(e) => {this.close(true, e)}}>
						{path}
						<Row>
							<Col md={4}>
								<Label>{i18n.t("modal.stream_name")}</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="text"
									value={this.state.name}
									placeholder={i18n.t("modal.name")}
									onChange={this.handleChangeName}
								/>
							</Col>
						</Row>
						<hr/>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("common.scope")}</Label>
							</Col>
							<Col md={8}>
								{scopeInput}
							</Col>
						</Row>
						<hr/>
						{recursive}
						<Row>
							<Col md={4}>
								<Label>{i18n.t("modal.play_as")}</Label>
							</Col>
							<Col md={8}>
								<FormControl componentClass="select"
                             placeholder="select"
                             value={this.state.type}
                             onChange={this.handleChangeType}>
									<option value="jukebox">{i18n.t("common.jukebox")}</option>
									<option value="webradio">{i18n.t("common.webradio")}</option>
								</FormControl>
							</Col>
						</Row>
						<Row>
							<Col md={12}>
								<hr/>
							</Col>
						</Row>
						{random}
						{icecast}
            <Row>
              <Col md={4}>
                <Label>{i18n.t("modal.stream_url")}</Label>
              </Col>
              <Col md={8}>
                <FormControl
                  type="text"
                  value={this.state.streamUrl}
                  placeholder={i18n.t("modal.stream_url_random")}
                  onChange={this.handleChangeUrl}
                  maxLength={32}
                />
              </Col>
            </Row>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("common.format")}</Label>
							</Col>
							<Col md={8}>
                {formatSelect}
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("common.channels")}</Label>
							</Col>
							<Col md={8}>
								<FormControl componentClass="select"
                             placeholder={i18n.t("common.select")}
                             value={this.state.channels}
                             onChange={this.handleChangeChannels}
                             disabled={this.state.channelsDisabled}>
									<option value="1">{i18n.t("common.channels_mono")}</option>
									<option value="2">{i18n.t("common.channels_stereo")}</option>
								</FormControl>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("common.bitrate")}</Label>
							</Col>
							<Col md={8}>
								<FormControl componentClass="select"
                             placeholder={i18n.t("common.select")}
                             value={this.state.bitrate}
                             onChange={this.handleChangeBitrate}
                             disabled={this.state.bitrateDisabled}>
									<option value="32000">{i18n.t("common.bitrate_bps", {bps: 32})}</option>
									<option value="96000">{i18n.t("common.bitrate_bps", {bps: 96})}</option>
									<option value="128000">{i18n.t("common.bitrate_bps", {bps: 128})}</option>
									<option value="192000">{i18n.t("common.bitrate_bps", {bps: 192})}</option>
									<option value="256000">{i18n.t("common.bitrate_bps", {bps: 256})}</option>
									<option value="320000">{i18n.t("common.bitrate_bps", {bps: 320})}</option>
								</FormControl>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("common.sample_rate")}</Label>
							</Col>
							<Col md={8}>
								<FormControl componentClass="select"
                             placeholder={i18n.t("common.select")}
                             value={this.state.sampleRate}
                             onChange={this.handleChangeSampleRate}
                             disabled={this.state.sampleRateDisabled}>
									<option value="8000">{i18n.t("common.sample_rate_khz", {khz: 8})}</option>
									<option value="11025">{i18n.t("common.sample_rate_khz", {khz: 11})}</option>
									<option value="22050">{i18n.t("common.sample_rate_khz", {khz: 22})}</option>
									<option value="32000">{i18n.t("common.sample_rate_khz", {khz: 32})}</option>
									<option value="44100">{i18n.t("common.sample_rate_khz", {khz: 44.1})}</option>
									<option value="48000">{i18n.t("common.sample_rate_khz", {khz: 48})}</option>
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
								<Label>{i18n.t("modal.play_now_current_player")}</Label>
							</Col>
							<Col md={8}>
                <Checkbox checked={this.state.playNow} onChange={this.handleChangePlayNow}>
                </Checkbox>
							</Col>
						</Row>
					</form>
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)}>{i18n.t("common.ok")}</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>{i18n.t("common.cancel")}</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalEditStream;
