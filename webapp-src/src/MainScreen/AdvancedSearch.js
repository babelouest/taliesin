import React, { Component } from 'react';
import { Row, Col, Button, ButtonGroup, FormGroup, ControlLabel, FormControl, Checkbox, DropdownButton, MenuItem, InputGroup, Image, Panel, PanelGroup } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import Datetime from 'react-datetime';
import 'react-datetime/css/react-datetime.css';
import StateStore from '../lib/StateStore';
import ModalMedia from '../Modal/ModalMedia';
import i18n from '../lib/i18n';

class AdvancedSearch extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			criteria: {
				query: "",
				type: "",
				data_source: StateStore.getState().dataSourceList.map((ds) => {return ds.name;}),
				tags: [],
				metrics: {},
				sort: "path",
				sort_direction: "asc",
				limit: 25,
				offset: 0
			},
			addMetrics: false,
			searchResult: [],
			searching: false,
			searchPanel: true,
			firstLoad: true,
			modalShow: false,
			modalMedia: false,
			modalTitle: ""
		};
		
		this.changeSelectDataSource = this.changeSelectDataSource.bind(this);
		this.handleChangeQuery = this.handleChangeQuery.bind(this);
		this.handleChangeType = this.handleChangeType.bind(this);
		this.handleAddTag = this.handleAddTag.bind(this);
		this.handleChangeTagKey = this.handleChangeTagKey.bind(this);
		this.handleSetTagKey = this.handleSetTagKey.bind(this);
		this.handleSetTagOperator = this.handleSetTagOperator.bind(this);
		this.handleChangeTagValue = this.handleChangeTagValue.bind(this);
		this.handleAddMetrics = this.handleAddMetrics.bind(this);
		this.handleSetMetrics = this.handleSetMetrics.bind(this);
		this.handleSetMetricsOperator = this.handleSetMetricsOperator.bind(this);
		this.handleChangeMetricsValue = this.handleChangeMetricsValue.bind(this);
		this.handleChangeMetricsValueMax = this.handleChangeMetricsValueMax.bind(this);
		this.handleRemoveMetrics = this.handleRemoveMetrics.bind(this);
		this.handleChangeMetricsPlayedAt = this.handleChangeMetricsPlayedAt.bind(this);
		this.handleChangeMetricsPlayedAtMax = this.handleChangeMetricsPlayedAtMax.bind(this);
		this.handleChangeMetricsLastSeen = this.handleChangeMetricsLastSeen.bind(this);
		this.handleChangeMetricsLastSeenMax = this.handleChangeMetricsLastSeenMax.bind(this);
		this.handleChangeMetricsLastUpdated = this.handleChangeMetricsLastUpdated.bind(this);
		this.handleChangeMetricsLastUpdatedMax = this.handleChangeMetricsLastUpdatedMax.bind(this);
		this.handleChangeSort = this.handleChangeSort.bind(this);
		this.handleChangeSortDirection = this.handleChangeSortDirection.bind(this);
		this.runSearch = this.runSearch.bind(this);
		this.openMedia = this.openMedia.bind(this);
		this.closeMedia = this.closeMedia.bind(this);
		this.navigate = this.navigate.bind(this);
		this.handleToggle = this.handleToggle.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			criteria: {
				query: "",
				type: "",
				data_source: StateStore.getState().dataSourceList.map((ds) => {return ds.name;}),
				tags: [],
				metrics: {},
				sort: "path",
				sort_direction: "asc",
				limit: 100,
				offset: 0
			},
			addMetrics: false,
			searchResult: [],
			searching: false,
			searchPanel: true,
			firstLoad: true,
			modalShow: false,
			modalMedia: false,
			modalTitle: ""
		});
	}
	
	getValidationState() {
		return "success";
	}
	
	handleChangeType(e) {
		var criteria = this.state.criteria;
		criteria.type = e.target.value;
		this.setState({criteria: criteria});
	}
	
	changeSelectDataSource(dataSource) {
		var criteria = this.state.criteria;
		var index = criteria.data_source.indexOf(dataSource);
		
		if (index === -1) {
			criteria.data_source.push(dataSource);
		} else {
			criteria.data_source.splice(index, 1);
		}
		this.setState({criteria: criteria});
	}
	
	handleChangeQuery(e) {
		var criteria = this.state.criteria;
		if (e.target.value !== "") {
			criteria.query = e.target.value;
		} else {
			delete criteria.query;
		}
		this.setState({criteria: criteria});
	}
	
	handleAddTag() {
		var criteria = this.state.criteria;
		criteria.tags.push({key: "", operator: "contains", value: ""});
		this.setState({criteria: criteria});
	}
	
	handleChangeTagKey(index, e) {
		var criteria = this.state.criteria;
		criteria.tags[index].key = e.target.value;
		this.setState({criteria: criteria});
	}
	
	handleSetTagKey(index, key) {
		var criteria = this.state.criteria;
		criteria.tags[index].key = key;
		this.setState({criteria: criteria});
	}
	
	handleSetTagOperator(index, e) {
		var criteria = this.state.criteria;
		criteria.tags[index].operator = e.target.value;
		this.setState({criteria: criteria});
	}
	
	handleChangeTagValue(index, e) {
		var criteria = this.state.criteria;
		criteria.tags[index].value = e.target.value;
		this.setState({criteria: criteria});
	}
	
	handleAddMetrics() {
		var metricsList = [<option key="0" value="">Select new metrics</option>], addMetrics;
		if (!this.state.criteria.metrics.nb_play) {
			metricsList.push(
				<option key="1" value="nb_play">{i18n.t("advanced_search.number_times_played")}</option>
			);
		}
		if (!this.state.criteria.metrics.played_at) {
			metricsList.push(
				<option key="2" value="played_at">{i18n.t("advanced_search.date_played")}</option>
			);
		}
		if (!this.state.criteria.metrics.last_seen) {
			metricsList.push(
				<option key="3" value="last_seen">{i18n.t("advanced_search.date_last_seen")}</option>
			);
		}
		if (!this.state.criteria.metrics.last_updated) {
			metricsList.push(
				<option key="4" value="last_updated">{i18n.t("advanced_search.date_last_updated")}</option>
			);
		}
		addMetrics = 
		<FormControl componentClass="select" placeholder={i18n.t("advanced_search.select")} onChange={(e) => this.handleSetMetrics(e)}>
		{metricsList}
		</FormControl>;
		this.setState({addMetrics: addMetrics});
	}
	
	handleSetMetrics(e) {
		var criteria = this.state.criteria;
		switch (e.target.value) {
			case "nb_play":
				criteria.metrics.nb_play = {operator: "equals", value: 0, value_max: 0};
				break;
			case "played_at":
				criteria.metrics.played_at = {operator: "equals", value: 0, value_max: 0};
				break;
			case "last_seen":
				criteria.metrics.last_seen = {operator: "equals", value: 0, value_max: 0};
				break;
			case "last_updated":
				criteria.metrics.last_updated = {operator: "equals", value: 0, value_max: 0};
				break;
			default:
				break;
		}
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleSetMetricsOperator(metrics, e) {
		var criteria = this.state.criteria;
		criteria.metrics[metrics].operator = e.target.value;
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleChangeMetricsValue(metrics, e) {
		var criteria = this.state.criteria;
		criteria.metrics[metrics].value = parseInt(e.target.value, 10);
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleChangeMetricsValueMax(metrics, e) {
		var criteria = this.state.criteria;
		criteria.metrics[metrics].value_max = parseInt(e.target.value, 10);
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleChangeMetricsPlayedAt(moment) {
		var criteria = this.state.criteria;
		criteria.metrics.played_at.value = Math.round(moment.valueOf() / 1000);
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleChangeMetricsPlayedAtMax(moment) {
		var criteria = this.state.criteria;
		criteria.metrics.played_at.value_max = Math.round(moment.valueOf() / 1000);
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleChangeMetricsLastSeen(moment) {
		var criteria = this.state.criteria;
		criteria.metrics.last_seen.value = Math.round(moment.valueOf() / 1000);
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleChangeMetricsLastSeenMax(moment) {
		var criteria = this.state.criteria;
		criteria.metrics.last_seen.value_max = Math.round(moment.valueOf() / 1000);
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleChangeMetricsLastUpdated(moment) {
		var criteria = this.state.criteria;
		criteria.metrics.last_updated.value = Math.round(moment.valueOf() / 1000);
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleChangeMetricsLastUpdatedMax(moment) {
		var criteria = this.state.criteria;
		criteria.metrics.last_updated.value_max = Math.round(moment.valueOf() / 1000);
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleRemoveMetrics(metrics) {
		var criteria = this.state.criteria;
		delete criteria.metrics[metrics];
		this.setState({criteria: criteria, addMetrics: false});
	}
	
	handleChangeSort(e) {
		var criteria = this.state.criteria;
		criteria.sort = e.target.value;
		this.setState({criteria: criteria});
	}
	
	handleChangeSortDirection(e) {
		var criteria = this.state.criteria;
		criteria.sort_direction = e.target.value;
		this.setState({criteria: criteria});
	}
	
	runSearch(e) {
		if (e) {
			e.preventDefault();
		}
		this.setState({searching: true, searchResult: [], firstLoad: false, searchPanel: false}, () => {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/search/", this.state.criteria)
			.then((result) => {
				this.setState({searchResult: result, searching: false}, () => {
					var searchResult = this.state.searchResult;
					searchResult.forEach((media) => {
						if (!media.cover) {
							StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(media.data_source) + "/browse/path/" + encodeURI(media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
							.then((cover) => {
								media.cover = cover;
								this.setState({searchResult: searchResult});
							});
						}
					});
				});
			})
			.fail(() => {
				this.setState({searchResult: [], searching: false});
			});
		});
	}
	
	navigate(delta) {
		var criteria = this.state.criteria;
		criteria.offset += delta;
		this.setState({criteria: criteria}, () => {
			this.runSearch();
		});
	}
	
	openMedia(media) {
		this.setState({modalShow: true, modalMedia: media, modalTitle: (media.tags.title||media.name)});
	}

	closeMedia() {
		this.setState({modalShow: false});
	}
  
  handleToggle() {
    this.setState({searchPanel: !this.state.searchPanel});
  }

	render() {
		var dataSourceList = [], tagList = [], metricsList = [], addMetrics, resultList = [], resultTable, resultEmpty;
		StateStore.getState().dataSourceList.forEach((dataSource, index) => {
			dataSourceList.push(
				<Checkbox key={index} inline checked={this.state.criteria.data_source.indexOf(dataSource.name)!==-1} onChange={() => this.changeSelectDataSource(dataSource.name)}>
					{dataSource.name}
				</Checkbox>
			);
		});
		this.state.criteria.tags.forEach((tag, index) => {
			var keyName = "";
			if (tag.key) {
				keyName = tag.key;
			}
			tagList.push(
				<Row key={index}>
					<Col xs={2}>
						<ControlLabel>Key</ControlLabel>
						<InputGroup>
							<DropdownButton title={keyName||"Select key"} id="keyList" className="form-control">
								<MenuItem onClick={() => this.handleSetTagKey(index, "title")}>{i18n.t("common.title")}</MenuItem>
								<MenuItem onClick={() => this.handleSetTagKey(index, "artist")}>{i18n.t("common.artist")}</MenuItem>
								<MenuItem onClick={() => this.handleSetTagKey(index, "album")}>{i18n.t("common.album")}</MenuItem>
								<MenuItem onClick={() => this.handleSetTagKey(index, "date")}>{i18n.t("common.year")}</MenuItem>
								<MenuItem onClick={() => this.handleSetTagKey(index, "genre")}>{i18n.t("common.genre")}</MenuItem>
								<MenuItem divider />
								<MenuItem>
									<FormControl
										type="text"
										value={keyName}
										placeholder={i18n.t("advanced_search.tag_key")}
										autoFocus
										onChange={(e) => {this.handleChangeTagKey(index, e)}}
									/>
								</MenuItem>
							</DropdownButton>
						</InputGroup>
					</Col>
					<Col xs={4}>
						<ControlLabel>{i18n.t("advanced_search.operator")}</ControlLabel>
						<FormControl componentClass="select" placeholder={i18n.t("advanced_search.select")} value={tag.operator} onChange={(e) => this.handleSetTagOperator(index, e)}>
							<option value="contains">{i18n.t("advanced_search.operator_contains")}</option>
							<option value="equals">{i18n.t("advanced_search.operator_equals")}</option>
							<option value="different">{i18n.t("advanced_search.operator_different")}</option>
							<option value="lower">{i18n.t("advanced_search.operator_lower")}</option>
							<option value="higher">{i18n.t("advanced_search.operator_higher")}</option>
							<option value="empty">{i18n.t("advanced_search.operator_empty")}</option>
						</FormControl>
					</Col>
					<Col xs={4}>
						<ControlLabel>{i18n.t("advanced_search.value")}</ControlLabel>
						<FormControl
							type="text"
							value={tag.name}
							placeholder={i18n.t("advanced_search.tag_name")}
							onChange={(e) => {this.handleChangeTagValue(index, e)}}
							disabled={tag.operator==="empty"}
						/>
					</Col>
					<Col xs={2}>
						<ControlLabel>{i18n.t("advanced_search.remove")}</ControlLabel>
						<InputGroup>
							<Button title={i18n.t("advanced_search.remove")} onClick={() => this.handleRemoveTag(index)}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
					</Col>
				</Row>
			);
		});
		if (this.state.addMetrics) {
			addMetrics = 
			<Row>
				<Col>
					<ControlLabel>{i18n.t("advanced_search.new_metric_value")}</ControlLabel>
					{this.state.addMetrics}
				</Col>
			</Row>;
		}
		var valueMax;
		if (this.state.criteria.metrics.nb_play) {
			valueMax = "";
			if (this.state.criteria.metrics.nb_play.operator === "between") {
				valueMax =
				<div>
					<ControlLabel>{i18n.t("advanced_search.value_max")}</ControlLabel>
					<FormControl
						type="text"
						value={this.state.criteria.metrics.nb_play.value_max}
						placeholder={i18n.t("advanced_search.value")}
						onChange={(e) => {this.handleChangeMetricsValueMax("nb_play", e)}}
					/>
				</div>
			}
			metricsList.push(
				<Row key={0}>
					<Col xs={2}>
						<span className="label label-info">
							{i18n.t("advanced_search.number_times_played")}
						</span>
					</Col>
					<Col xs={4}>
						<ControlLabel>{i18n.t("advanced_search.operator")}</ControlLabel>
						<FormControl componentClass="select" placeholder={i18n.t("advanced_search.select")} value={this.state.criteria.metrics.nb_play.operator} onChange={(e) => this.handleSetMetricsOperator("nb_play", e)}>
							<option value="contains">{i18n.t("advanced_search.operator_contains")}</option>
							<option value="equals">{i18n.t("advanced_search.operator_equals")}</option>
							<option value="different">{i18n.t("advanced_search.operator_different")}</option>
							<option value="lower">{i18n.t("advanced_search.operator_lower")}</option>
							<option value="higher">{i18n.t("advanced_search.operator_higher")}</option>
							<option value="empty">{i18n.t("advanced_search.operator_empty")}</option>
						</FormControl>
					</Col>
					<Col xs={4}>
						<ControlLabel>{i18n.t("advanced_search.value")}</ControlLabel>
						<FormControl
							type="text"
							value={this.state.criteria.metrics.nb_play.value}
							placeholder={i18n.t("advanced_search.value")}
							onChange={(e) => {this.handleChangeMetricsValue("nb_play", e)}}
						/>
						{valueMax}
					</Col>
					<Col xs={2}>
						<ControlLabel>{i18n.t("advanced_search.remove")}</ControlLabel>
						<InputGroup>
							<Button title={i18n.t("advanced_search.remove")} onClick={() => this.handleRemoveMetrics("nb_play")}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
					</Col>
				</Row>
			);
		}
		if (this.state.criteria.metrics.played_at) {
			valueMax = "";
			if (this.state.criteria.metrics.played_at.operator === "between") {
				valueMax =
				<div>
					<ControlLabel>Value max</ControlLabel>
					<Datetime value={this.state.criteria.metrics.played_at.value_max*1000} onChange={this.handleChangeMetricsPlayedAtMax}/>
				</div>
			}
			metricsList.push(
				<Row key={1}>
					<Col xs={2}>
						<span className="label label-info">
							{i18n.t("advanced_search.played_at")}
						</span>
					</Col>
					<Col xs={4}>
						<ControlLabel>{i18n.t("advanced_search.operator")}</ControlLabel>
						<FormControl componentClass="select" placeholder={i18n.t("advanced_search.select")} value={this.state.criteria.metrics.played_at.operator} onChange={(e) => this.handleSetMetricsOperator("played_at", e)}>
							<option value="equals">{i18n.t("advanced_search.operator_on")}</option>
							<option value="different">{i18n.t("advanced_search.operator_not_on")}</option>
							<option value="lower">{i18n.t("advanced_search.operator_before")}</option>
							<option value="higher">{i18n.t("advanced_search.operator_after")}</option>
							<option value="between">{i18n.t("advanced_search.operator_between")}</option>
						</FormControl>
					</Col>
					<Col xs={4}>
						<ControlLabel>{i18n.t("advanced_search.value")}</ControlLabel>
						<Datetime value={this.state.criteria.metrics.played_at.value*1000} onChange={this.handleChangeMetricsPlayedAt}/>
						{valueMax}
					</Col>
					<Col xs={2}>
						<ControlLabel>{i18n.t("advanced_search.remove")}</ControlLabel>
						<InputGroup>
							<Button title={i18n.t("advanced_search.remove")} onClick={() => this.handleRemoveMetrics("played_at")}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
					</Col>
				</Row>
			);
		}
		if (this.state.criteria.metrics.last_seen) {
			valueMax = "";
			if (this.state.criteria.metrics.last_seen.operator === "between") {
				valueMax =
				<div>
					<ControlLabel>{i18n.t("advanced_search.value_max")}</ControlLabel>
					<Datetime value={this.state.criteria.metrics.last_seen.value_max*1000} onChange={this.handleChangeMetricsLastSeenMax}/>
				</div>
			}
			metricsList.push(
				<Row key={2}>
					<Col xs={2}>
						<span className="label label-info">
							{i18n.t("advanced_search.last_seen")}
						</span>
					</Col>
					<Col xs={4}>
						<ControlLabel>{i18n.t("advanced_search.operator")}</ControlLabel>
						<FormControl componentClass="select" placeholder={i18n.t("advanced_search.select")} value={this.state.criteria.metrics.last_seen.operator} onChange={(e) => this.handleSetMetricsOperator("last_seen", e)}>
							<option value="equals">{i18n.t("advanced_search.operator_on")}</option>
							<option value="different">{i18n.t("advanced_search.operator_not_on")}</option>
							<option value="lower">{i18n.t("advanced_search.operator_before")}</option>
							<option value="higher">{i18n.t("advanced_search.operator_after")}</option>
							<option value="between">{i18n.t("advanced_search.operator_between")}</option>
						</FormControl>
					</Col>
					<Col xs={4}>
						<ControlLabel>Value</ControlLabel>
						<Datetime value={this.state.criteria.metrics.last_seen.value*1000} onChange={this.handleChangeMetricsLastSeen}/>
						{valueMax}
					</Col>
					<Col xs={2}>
						<ControlLabel>{i18n.t("advanced_search.remove")}</ControlLabel>
						<InputGroup>
							<Button title={i18n.t("advanced_search.remove")} onClick={() => this.handleRemoveMetrics("last_seen")}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
					</Col>
				</Row>
			);
		}
		if (this.state.criteria.metrics.last_updated) {
			valueMax = "";
			if (this.state.criteria.metrics.last_updated.operator === "between") {
				valueMax =
				<div>
					<ControlLabel>{i18n.t("advanced_search.value_max")}</ControlLabel>
					<Datetime value={this.state.criteria.metrics.last_updated.value_max*1000} onChange={this.handleChangeMetricsLastUpdatedMax}/>
				</div>
			}
			metricsList.push(
				<Row key={3}>
					<Col xs={2}>
						<span className="label label-info">
							{i18n.t("advanced_search.last_updated")}
						</span>
					</Col>
					<Col xs={4}>
						<ControlLabel>{i18n.t("advanced_search.operator")}</ControlLabel>
						<FormControl componentClass="select" placeholder={i18n.t("advanced_search.select")} value={this.state.criteria.metrics.last_updated.operator} onChange={(e) => this.handleSetMetricsOperator("last_updated", e)}>
							<option value="equals">{i18n.t("advanced_search.operator_on")}</option>
							<option value="different">{i18n.t("advanced_search.operator_not_on")}</option>
							<option value="lower">{i18n.t("advanced_search.operator_before")}</option>
							<option value="higher">{i18n.t("advanced_search.operator_after")}</option>
							<option value="between">{i18n.t("advanced_search.operator_between")}</option>
						</FormControl>
					</Col>
					<Col xs={4}>
						<ControlLabel>{i18n.t("advanced_search.value")}</ControlLabel>
						<Datetime value={this.state.criteria.metrics.last_updated.value*1000} onChange={this.handleChangeMetricsLastUpdated}/>
						{valueMax}
					</Col>
					<Col xs={2}>
						<ControlLabel>{i18n.t("advanced_search.remove")}</ControlLabel>
						<InputGroup>
							<Button title={i18n.t("advanced_search.remove")} onClick={() => this.handleRemoveMetrics("last_updated")}>
								<FontAwesome name="times"/>
							</Button>
						</InputGroup>
					</Col>
				</Row>
			);
		}
		this.state.searchResult.forEach((result, index) => {
			var cover;
			if (result.cover) {
				cover = <Image src={"data:image/jpeg;base64," + result.cover} responsive style={{maxWidth: "100px", maxHeight: "100px"}}/>
			}
			resultList.push(
				<tr key={index}>
					<td>
						<a role="button" onClick={() => {this.openMedia(result)}}>
							{result.data_source}
						</a>
					</td>
					<td>
						<a role="button" onClick={() => {this.openMedia(result)}}>
							{result.tags.title?result.tags.title:result.name}
						</a>
					</td>
					<td>
						<a role="button" onClick={() => {this.openMedia(result)}}>
							{result.path}
						</a>
					</td>
					<td>
						<a role="button" onClick={() => {this.openMedia(result)}}>
							<span className="space-after">{result.nb_play}</span>{i18n.t("common.times")}
						</a>
					</td>
					<td>
						<a role="button" onClick={() => {this.openMedia(result)}}>
							{cover}
						</a>
					</td>
				</tr>
			);
		});
		resultEmpty =
			<tr>
				<td colspan="4">
					<strong>{i18n.t("advanced_search.no_result_found")}</strong>
				</td>
			</tr>;
		if (!this.state.firstLoad) {
			if (!this.state.searching) {
				resultTable =
					<div>
						<Row style={{marginTop: "10px"}}>
							<Col md={12} sm={12} xs={12} className="text-right">
								<ButtonGroup>
									<Button disabled={!this.state.criteria.offset} onClick={() => {this.navigate(-25)}}>
										<span className="hidden-sm hidden-xs">{i18n.t("common.previous_page")}</span>
										<span className="visible-sm visible-xs">
											<FontAwesome name="chevron-left" />
										</span>
									</Button>
									<Button disabled={(this.state.offset + this.state.criteria.limit) >= resultList.length} onClick={() => {this.navigate(25)}}>
										<span className="hidden-sm hidden-xs">{i18n.t("common.next_page")}</span>
										<span className="visible-sm visible-xs">
											<FontAwesome name="chevron-right" />
										</span>
									</Button>
								</ButtonGroup>
							</Col>
						</Row>
						<table className="table table-striped table-hover">
							<thead>
								<tr>
									<th>
										{i18n.t("common.data_source")}
									</th>
									<th>
										{i18n.t("common.name")}
									</th>
									<th>
										{i18n.t("common.path")}
									</th>
									<th>
										{i18n.t("common.nb_play")}
									</th>
									<th>
										{i18n.t("common.cover")}
									</th>
								</tr>
							</thead>
							<tbody>
								{resultList.length?resultList:resultEmpty}
							</tbody>
						</table>
						<Row style={{marginTop: "10px"}}>
							<Col md={12} sm={12} xs={12} className="text-right">
								<ButtonGroup>
									<Button disabled={!this.state.criteria.offset} onClick={() => {this.navigate(-25)}}>
										<span className="hidden-sm hidden-xs">{i18n.t("common.previous_page")}</span>
										<span className="visible-sm visible-xs">
											<FontAwesome name="chevron-left" />
										</span>
									</Button>
									<Button disabled={(this.state.offset + this.state.criteria.limit) >= resultList.length} onClick={() => {this.navigate(25)}}>
										<span className="hidden-sm hidden-xs">{i18n.t("common.next_page")}</span>
										<span className="visible-sm visible-xs">
											<FontAwesome name="chevron-right" />
										</span>
									</Button>
								</ButtonGroup>
							</Col>
						</Row>
					</div>;
			} else {
				resultTable =
					<div>
						{i18n.t("advanced_search.searching")} <FontAwesome name="spinner" spin />
					</div>
			}
		}
		return (
			<div>
				<form onSubmit={(e) => {this.runSearch(e)}}>
					<FormGroup
						controlId="formBasicText"
						validationState={this.getValidationState()}
					>
            <PanelGroup id="myPanel">
              <Panel expanded={this.state.searchPanel} onToggle={this.handleToggle}>
                <Panel.Heading>
                  <Panel.Title toggle>
                    {i18n.t("advanced_search.search_query")}
                  </Panel.Title>
                </Panel.Heading>
                <Panel.Collapse>
                  <Panel.Body>
                    <Row>
                      <ControlLabel>{i18n.t("advanced_search.search_query")}</ControlLabel>
                      <FormControl
                        type="text"
                        value={this.state.criteria.query}
                        placeholder={i18n.t("advanced_search.text_to_search")}
                        onChange={this.handleChangeQuery}
                      />
                    </Row>
                    <Row>
                      <ControlLabel>{i18n.t("advanced_search.media_type")}</ControlLabel>
                      <FormControl componentClass="select" placeholder={i18n.t("advanced_search.select")} value={this.state.criteria.type} onChange={this.handleChangeType}>
                        <option value="">{i18n.t("advanced_search.type_all")}</option>
                        <option value="audio">{i18n.t("advanced_search.type_audio")}</option>
                        <option value="video">{i18n.t("advanced_search.type_video")}</option>
                        <option value="image">{i18n.t("advanced_search.type_image")}</option>
                        <option value="subtitle">{i18n.t("advanced_search.type_subtitle")}</option>
                        <option value="unknown">{i18n.t("advanced_search.type_unknown")}</option>
                      </FormControl>
                    </Row>
                    <Row>
                      <hr/>
                    </Row>
                    <Row>
                      <h3>{i18n.t("common.data_source")}</h3>
                    </Row>
                    <Row>
                      {dataSourceList}
                    </Row>
                    <Row>
                      <hr/>
                    </Row>
                    <Row>
                      <h3>{i18n.t("common.tags")}</h3>
                    </Row>
                    <Row>
                      {tagList}
                    </Row>
                    <Row style={{paddingTop: "10px"}}>
                      <Button title="Add tag" onClick={this.handleAddTag}>
                        <FontAwesome name="plus" />
                      </Button>
                    </Row>
                    <Row>
                      <hr/>
                    </Row>
                    <Row>
                      <h3>{i18n.t("advanced_search.metrics")}</h3>
                    </Row>
                    {metricsList}
                    {addMetrics}
                    <Row style={{paddingTop: "10px"}}>
                      <Button title={i18n.t("advanced_search.add_tag")} onClick={this.handleAddMetrics}>
                        <FontAwesome name="plus" />
                      </Button>
                    </Row>
                    <Row>
                      <hr/>
                    </Row>
                    <Row>
                      <h3>{i18n.t("advanced_search.sort")}</h3>
                    </Row>
                    <Row style={{paddingTop: "10px"}}>
                      <FormControl componentClass="select" placeholder={i18n.t("advanced_search.sort_by")} value={this.state.criteria.sort} onChange={this.handleChangeSort}>
                        <option value="path">{i18n.t("common.path")}</option>
                        <option value="name">{i18n.t("common.name")}</option>
                        <option value="last_updated">{i18n.t("advanced_search.last_updated")}</option>
                        <option value="last_played">{i18n.t("advanced_search.last_seen")}</option>
                        <option value="nb_play">{i18n.t("advanced_search.nb_play")}</option>
                        <option value="random">{i18n.t("advanced_search.random")}</option>
                      </FormControl>
                      <FormControl componentClass="select" placeholder={i18n.t("advanced_search.sort_direction")} value={this.state.criteria.sort_direction} onChange={this.handleChangeSortDirection}>
                        <option value="asc">{i18n.t("advanced_search.asc")}</option>
                        <option value="desc">{i18n.t("advanced_search.desc")}</option>
                      </FormControl>
                    </Row>
                    <Row>
                      <hr/>
                    </Row>
                  </Panel.Body>
                </Panel.Collapse>
              </Panel>
            </PanelGroup>
						<Row>
							<Button title={i18n.t("advanced_search.search")} onClick={this.runSearch}>
								{i18n.t("advanced_search.search")} <FontAwesome name="search" />
							</Button>
						</Row>
					</FormGroup>
				</form>
				{resultTable}
				<ModalMedia show={this.state.modalShow} media={this.state.modalMedia} title={this.state.modalTitle} onClose={this.closeMedia} />
			</div>
		);
	}
}

export default AdvancedSearch;
