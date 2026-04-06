import { Component, OnInit, Input } from '@angular/core';
import { ScheduleOrProgram } from '../../../services/interfaces/program.interface';
import { UtilityService } from '../../../services/utility.service';
import { TranslatePipe } from '@ngx-translate/core';
import { TooltipModule } from 'primeng/tooltip';


@Component({
    selector: 'app-status-scheduled',
    templateUrl: './scheduled.component.html',
    styleUrls: ['./scheduled.component.css', '../../status.component.css'],
    imports: [TooltipModule, TranslatePipe]
})
export class ScheduledComponent implements OnInit {
    @Input() scheduled?: ScheduleOrProgram[];

    constructor(public utility: UtilityService) { }

    ngOnInit(): void {
    }


}
