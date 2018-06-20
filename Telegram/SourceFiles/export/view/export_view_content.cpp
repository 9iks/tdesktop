/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "export/view/export_view_content.h"

#include "lang/lang_keys.h"
#include "layout.h"

namespace Export {
namespace View {

Content ContentFromState(const ProcessingState &state) {
	using Step = ProcessingState::Step;

	auto result = Content();
	const auto push = [&](
			const QString &id,
			const QString &label,
			const QString &info,
			float64 progress) {
		result.rows.push_back({ id, label, info, progress });
	};
	const auto pushMain = [&](const QString &label) {
		const auto info = (state.entityCount > 0)
			? (QString::number(state.entityIndex)
				+ " / "
				+ QString::number(state.entityCount))
			: QString();
		if (!state.substepsTotal) {
			push("main", label, info, 0.);
			return;
		}
		const auto substepsTotal = state.substepsTotal;
		const auto step = static_cast<int>(state.step);
		const auto done = state.substepsPassed;
		const auto add = state.substepsNow;
		const auto doneProgress = done / float64(substepsTotal);
		const auto addProgress = (state.entityCount > 0)
			? ((float64(add) * state.entityIndex)
				/ (float64(substepsTotal) * state.entityCount))
			: 0.;
		push("main", label, info, doneProgress + addProgress);
	};
	const auto pushBytes = [&](const QString &id, const QString &label) {
		if (!state.bytesCount) {
			return;
		}
		const auto progress = state.bytesLoaded / float64(state.bytesCount);
		const auto info = formatDownloadText(
			state.bytesLoaded,
			state.bytesCount);
		push(id, label, info, progress);
	};
	switch (state.step) {
	case Step::Initializing:
		pushMain(lang(lng_export_state_initializing));
		break;
	case Step::LeftChannelsList:
	case Step::DialogsList:
		pushMain(lang(lng_export_state_chats_list));
		break;
	case Step::PersonalInfo:
		pushMain(lang(lng_export_option_info));
		break;
	case Step::Userpics:
		pushMain(lang(lng_export_state_userpics));
		pushBytes(
			"userpic" + QString::number(state.entityIndex),
			state.bytesName);
		break;
	case Step::Contacts:
		pushMain(lang(lng_export_option_contacts));
		break;
	case Step::Sessions:
		pushMain(lang(lng_export_option_sessions));
		break;
	case Step::LeftChannels:
	case Step::Dialogs:
		pushMain(lang(lng_export_state_chats));
		if (state.itemCount > 0) {
			push(
				"chat" + QString::number(state.entityIndex),
				(state.entityName.isEmpty()
					? lang(lng_deleted)
					: state.entityName),
				(QString::number(state.itemIndex)
					+ " / "
					+ QString::number(state.itemCount)),
				state.itemIndex / float64(state.itemCount));
		}
		pushBytes(
			("file"
				+ QString::number(state.entityIndex)
				+ '_'
				+ QString::number(state.itemIndex)),
			state.bytesName);
		break;
	default: Unexpected("Step in ContentFromState.");
	}
	return result;
}

} // namespace View
} // namespace Export