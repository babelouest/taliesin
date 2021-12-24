import React, { Component } from 'react';
import { Button, Modal, FormControl, FormGroup, Row, Col, Label } from 'react-bootstrap';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class ModalEditPlaylist extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			show: props.show, 
			add: props.add,
			playlist: props.add?{name: "", description: "", cover: undefined, scope: "me"}:props.playlist, 
			iconPath: "",
			onCloseCb: props.onCloseCb
		};
		
		this.close = this.close.bind(this);
		this.handleChangeName = this.handleChangeName.bind(this);
		this.handleChangeDescription = this.handleChangeDescription.bind(this);
		this.handleChangeScope = this.handleChangeScope.bind(this);
		this.getNameValidationState = this.getNameValidationState.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			show: nextProps.show, 
			add: nextProps.add,
			playlist: nextProps.add?{name: "", description: "", scope: "me"}:nextProps.playlist, 
			iconPath: "",
			onCloseCb: nextProps.onCloseCb,
			nameValidateMessage: ""
		});
	}

	close(result, e) {
		if (e) {
			e.preventDefault();
		}
		if (result && this.state.onCloseCb) {
			this.setState({ show: false }, () => {
				this.state.onCloseCb(true, this.state.playlist, this.state.add);
			});
		} else {
			this.setState({ show: false }, () => {
				this.state.onCloseCb(false, false, false);
			});
		}
	}
	
	getNameValidationState() {
		if (this.state.add) {
			if (this.state.playlist.name) {
				var found = StateStore.getState().playlists.find((playlist) => {
					return playlist.name === this.state.playlist.name;
				})
				if (found) {
					return "error";
				} else {
					return "success";
				}
			} else {
				return "error";
			}
		} else {
			return "success";
		}
	}
	
	handleChangeName(e) {
		var playlist = this.state.playlist;
		playlist.name = e.target.value;
		this.setState({ playlist: playlist });
	}

	handleChangeDescription(e) {
		var playlist = this.state.playlist;
		playlist.description = e.target.value;
		this.setState({ playlist: playlist });
	}
	
	handleChangeScope(e) {
		var playlist = this.state.playlist;
		playlist.scope = e.target.value;
		this.setState({ playlist: playlist });
	}
	
	render() {
		var title = this.state.add?i18n.t("modal.title_add_playlist"):i18n.t("modal.title_edit_playlist", {playlist: this.state.playlist.name||""});
		var scopeInput;
		if (StateStore.getState().profile.isAdmin && this.state.add) {
			scopeInput =
				<FormControl
					value={this.state.playlist.scope}
					onChange={this.handleChangeScope}
					disabled={!this.state.add}
					componentClass="select"
					placeholder={i18n.t("common.select")}
				>
					<option value={"me"}>{i18n.t("common.scope_me")}</option>
					<option value={"all"}>{i18n.t("common.scope_all")}</option>
				</FormControl>;
		} else {
			if (!this.state.add) {
				scopeInput = <span>{this.state.playlist.scope==="me"?i18n.t("common.scope_me"):i18n.t("common.scope_all")}</span>;
			} else {
				scopeInput = <span>{i18n.t("common.scope_me")}</span>;
			}
		}
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>{title}</Modal.Title>
				</Modal.Header>

				<Modal.Body>
					<form onSubmit={(e) => {this.close(true, e)}}>
						<FormGroup
							controlId="formName"
							validationState={this.getNameValidationState()}
						>
							<Row>
								<Col md={4}>
									<Label>{i18n.t("common.name")}</Label>
								</Col>
								<Col md={8}>
									<FormControl
										type="text"
										value={this.state.playlist.name}
										disabled={!this.state.add}
										placeholder={i18n.t("common.name")}
										onChange={this.handleChangeName}
									/>
								</Col>
							</Row>
						</FormGroup>
						<Row>
							<Col md={12}>
								<hr/>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("common.description")}</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="text"
									value={this.state.playlist.description}
									placeholder={i18n.t("common.description")}
									onChange={this.handleChangeDescription}
								/>
							</Col>
						</Row>
						<Row>
							<Col md={12}>
								<hr/>
							</Col>
						</Row>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("common.scope")}</Label>
							</Col>
							<Col md={8}>
								{scopeInput}
							</Col>
						</Row>
					</form>
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)} disabled={this.getNameValidationState() !== "success"}>{i18n.t("common.ok")}</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>{i18n.t("common.cancel")}</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalEditPlaylist;
